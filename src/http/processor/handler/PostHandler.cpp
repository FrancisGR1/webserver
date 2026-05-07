#include <fcntl.h>
#include <unistd.h>

#include "core/MimeTypes.hpp"
#include "core/ResourceLocker.hpp"
#include "core/constants.hpp"
#include "core/contracts.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "http/http_utils.hpp"
#include "http/processor/RequestConfig.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/PostHandler.hpp"
#include "http/request/Request.hpp"

PostHandler::PostHandler(const Request& request, const RequestContext& ctx)
    : m_request(request)
    , m_ctx(ctx)
    , m_done(false)
    , m_post("")
    , m_offset(0)
    , m_current_part(0)
{
    Logger::verbose("PostHandler: constructor");
}

PostHandler::~PostHandler()
{
    Logger::trace("PostHandler: destructor");
    if (m_post.fd > -1)
    {
        Logger::trace("PostHandler: close '%d'", m_post.fd);
        m_post.close();
    }
}

static void expect_uploadable(
    const Request& request,
    const RequestConfig& config,
    const Path& upload_dir,
    const RequestContext& ctx)
{
    if (request.body().size() > config.max_body_size())
        http_utils::throw_content_too_large(ctx);
    if (!upload_dir.exists)
        http_utils::throw_not_found(upload_dir, ctx);
    if (!upload_dir.is_directory)
        http_utils::throw_conflict_delete(upload_dir, ctx);
    if (!config.allows_upload() || !upload_dir.can_write || !upload_dir.can_execute)
        http_utils::throw_forbidden_cant_upload(upload_dir, ctx);
}

void PostHandler::process()
{
    const RequestConfig& config = m_ctx.config();

    if (!config.allows_method("POST"))
        http_utils::throw_method_not_allowed("POST", m_ctx);

    if (config.is_redirected())
    {
        m_response.set_status(StatusCode::MovedPermanently);
        m_response.set_header(
            "Location", make_uri()); //@TODO @ABORT @IMPORTANT: m_upload_filename is ""! will set contract off
        m_response.set_header("Connection", "close");
        m_response.set_header("Date", utils::http_date());
        m_done = true;
    }
    else if (config.allows_upload())
    {
        if (m_request.multipart_body().empty())
            upload_request_body();
        else
            upload_multipart_body();
    }
    else // can't upload
    {
        http_utils::throw_forbidden_cant_upload(m_ctx.config().path(), m_ctx);
    }
}

bool PostHandler::done() const
{
    return m_done;
}

const Response& PostHandler::response() const
{
    return m_response;
}

std::vector<EventAction> PostHandler::give_events()
{
    return std::vector<EventAction>();
}

std::vector<Path> PostHandler::upload_paths() const
{
    std::vector<Path> result;
    if (m_post.exists || m_post.fd != -1)
        result.push_back(m_post);
    return result;
}

std::string PostHandler::make_uri() const
{
    REQUIRE(m_ctx.config().location() != NULL, "Location must not be null");
    REQUIRE(m_upload_filename != "", "Upload file must be a non empty string");

    std::string uri = utils::join_paths(m_ctx.config().location()->name, m_upload_filename);
    Logger::debug("PostHandler: uri: '%s'", uri.c_str());
    return uri;
}

std::string PostHandler::make_file_name() const
{
    static unsigned long long uploaded_file_index;
    bool req_has_filename = true;
    std::string file_name = "";

    // check for filename in request
    if (utils::contains(m_request.headers(), "Content-Disposition"))
    {
        const std::string& value = m_request.headers().at("Content-Disposition");
        size_t pos = value.find("filename");
        if (pos == std::string::npos)
        {
            req_has_filename = false;
        }
        else
        // filename parameter exists so extract it
        {
            size_t eq = value.find('=', pos);
            if (eq == std::string::npos)
            // no value
            {
                req_has_filename = false;
            }
            else
            // has value
            {
                file_name = value.substr(eq + 1); // everything after '='
                // remove quotes
                if (!file_name.empty() && file_name[0] == '"')
                    file_name = file_name.substr(1);
                if (!file_name.empty() && file_name[file_name.size() - 1] == '"')
                    file_name.erase(file_name.size() - 1);
            }
        }
    }
    else
    {
        req_has_filename = false;
    }

    if (!req_has_filename)
    {
        std::string extension = ".data";
        if (utils::contains(m_request.headers(), "Content-Type"))
        {
            const std::string& mime = m_request.headers().at("Content-Type");
            extension = MimeTypes::from_mime(mime);
        }

        // default: timestamp + index + extension
        file_name = utils::to_string(utils::timestamp()) + "_" + utils::to_string(uploaded_file_index++) + extension;
    }

    Logger::debug("PostHandler: file name: '%s'", file_name.c_str());
    return file_name;
}

void PostHandler::upload_request_body()
{
    if (m_post.fd == -1) // set upload dir
    {
        Path upload_dir = m_ctx.config().path();
        expect_uploadable(m_request, m_ctx.config(), upload_dir, m_ctx);

        // create a name for the new file to be uploaded
        m_upload_filename = make_file_name();
        // make upload location
        m_upload_uri = make_uri();
        // make upload real path
        m_post = utils::join_paths(upload_dir.raw, m_upload_filename);
        //  open fd and store in events
        m_post.open(O_WRONLY | O_CREAT | O_TRUNC, 0644);
        // occupy
        ResourceLocker::lock(m_post);
        // push
        // m_upload_paths.push_back(m_post);
        if (m_post.fd <= 2)
        {
            ResourceLocker::unlock(m_post);
            http_utils::throw_internal_server_error_failed_upload(upload_dir, m_ctx);
        }

        Logger::debug("PostHandler: write to fd=%d '%s'", m_post.fd, m_post.c_str());
    }

    if (!m_done)
    {
        size_t to_write = (m_request.body().size() - m_offset) > constants::write_chunk_size
                              ? constants::write_chunk_size
                              : m_request.body().size() - m_offset;
        ssize_t written = ::write(m_post.fd, m_request.body().c_str() + m_offset, to_write);
        if (written == -1)
        {
            ResourceLocker::unlock(m_post);
            m_post.close();
            http_utils::throw_internal_server_error_failed_upload(m_post, m_ctx);
        }

        if (written > 0)
            m_offset += written;

        if (written == 0 || m_offset == static_cast<ssize_t>(m_request.body().size()))
            m_done = true;

        Logger::trace("PostHandler: write %zu(%zu)", written, m_offset);
    }

    if (m_done)
    {
        Logger::trace("PostHandler: finished writing fd=%d", m_post.fd);

        // status
        m_response.set_status(StatusCode::Created);

        // body
        std::string json = "{"
                           "\"status\": \"success\","
                           "\"filename\": \"" +
                           m_upload_uri +
                           "\","
                           "\"size\": " +
                           utils::to_string(m_offset) + "}";
        m_response.set_body_as_str(json);

        // headers
        m_response.set_header("Location", m_upload_uri);
        m_response.set_header("Connection", "close"); // @NOTE: HTTP1.0 closes by default;
        m_response.set_header("Date", utils::http_date());
        m_response.set_header("Content-Type", "application/json");
        m_response.set_header("Content-Length", utils::to_string(json.size()));

        // close posted file
        m_post.close();

        // unoccupy
        ResourceLocker::unlock(m_post);
    }
}

void PostHandler::upload_multipart_body()
{
    const std::vector<MultiPartBody>& parts = m_request.multipart_body();

    while (m_current_part < parts.size())
    {
        const MultiPartBody& part = parts[m_current_part];

        if (m_post.fd == -1)
        {
            Path upload_dir = m_ctx.config().path();
            expect_uploadable(m_request, m_ctx.config(), upload_dir, m_ctx);

            // use part filename if available, else generate one
            if (!part.filename.empty())
                m_upload_filename = part.filename;
            else
                m_upload_filename = part.name + ".data";

            // uri
            m_upload_uri = make_uri();
            // save
            m_post = utils::join_paths(upload_dir.raw, m_upload_filename);
            // open
            m_post.open(O_WRONLY | O_CREAT | O_TRUNC, 0644);
            // lock
            ResourceLocker::lock(m_post);
            if (m_post.fd <= 2)
                http_utils::throw_internal_server_error_failed_upload(m_post, m_ctx);

            m_offset = 0;
            Logger::debug("PostHandler: multipart write to fd=%d '%s'", m_post.fd, m_post.c_str());
        }

        // write chunk
        size_t to_write = (part.body.size() - m_offset) > constants::write_chunk_size ? constants::write_chunk_size
                                                                                      : part.body.size() - m_offset;
        ssize_t written = ::write(m_post.fd, part.body.c_str() + m_offset, to_write);
        if (written > 0)
            m_offset += written;

        if (written == -1 || m_offset == static_cast<ssize_t>(part.body.size()))
        {
            // this part is over
            m_post.close();
            m_offset = 0;
            m_current_part++;
            // unoccupy
            ResourceLocker::unlock(m_post);
        }
        else // not done yet, come back next process() call
        {
            return;
        }
    }

    // all parts written
    m_done = true;

    m_response.set_status(StatusCode::Created);
    m_response.set_header("Connection", "close");
    m_response.set_header("Date", utils::http_date());

    std::string json = "{\"status\": \"success\", \"uploaded\": " + utils::to_string(parts.size()) + "}";
    m_response.set_body_as_str(json);
    m_response.set_header("Content-Type", "application/json");
    m_response.set_header("Content-Length", utils::to_string(json.size()));
}
