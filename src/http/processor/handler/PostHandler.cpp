#include <fcntl.h>
#include <unistd.h>

#include "PostHandler.hpp"
#include "core/constants.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "http/http_utils.hpp"
#include "http/processor/RequestConfig.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/request/Request.hpp"

unsigned long long PostHandler::m_uploaded_file_index;

PostHandler::PostHandler(const Request& request, const RequestContext& ctx)
    : m_request(request)
    , m_ctx(ctx)
    , m_done(false)
    , m_cgi(request, ctx, constants::cgi_timeout)
    , m_upload("")
    , m_fd(-1)
{
    Logger::trace("PostHandler: constructor");
}

static void expect_uploadable(
    const Request& request,
    const RequestConfig& config,
    const Path& upload_dir,
    const RequestContext& ctx)
{
    if (!upload_dir.exists)
        http_utils::throw_internal_server_error_cant_upload(ctx);
    if (request.body().size() > config.max_body_size())
        http_utils::throw_content_too_large(ctx);
    if (!upload_dir.exists)
        http_utils::throw_internal_server_error_doesnt_exist(upload_dir, ctx);
    if (!upload_dir.is_directory)
        http_utils::throw_internal_server_error_not_a_directory(upload_dir, ctx);
    if (!upload_dir.can_write || !upload_dir.can_execute)
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
        m_response.set_header("Location", config.redirection().raw_path);
        m_response.set_header("Connection", "close");
        m_response.set_header("Date", utils::http_date());
        m_done = true;
    }
    else if (config.is_cgi())
    {
        m_cgi.process();
        if (m_cgi.done())
            m_done = true;
        return;
    }
    else if (config.allows_upload())
    {
        if (m_fd == -1) // set upload dir
        {
            Path upload_dir = m_ctx.config().path();
            expect_uploadable(m_request, config, upload_dir, m_ctx);

            // create a name for the new file to be uploaded
            std::string file_name = utils::to_string(m_uploaded_file_index++) + ".data";
            // make upload real path
            m_upload = utils::join_paths(upload_dir.raw, file_name);
            //@TODO add fd to event pool
            m_fd = open(m_upload.raw.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (m_fd <= 2)
            {
                http_utils::throw_internal_server_error_failed_upload(upload_dir, m_ctx);
            }

            Logger::debug("PostHandler: write to fd=%d '%s'", m_fd, m_upload.raw.c_str());
        }

        if (!m_done)
        {
            size_t to_write = (m_request.body().size() - m_offset) > constants::write_chunk_size
                                  ? constants::write_chunk_size
                                  : m_request.body().size() - m_offset;
            ssize_t written = ::write(m_fd, m_request.body().c_str() + m_offset, to_write);
            if (written >= 0)
                m_offset += written;
            //@TODO apanhar erro em caso de -1
            if (m_offset == static_cast<ssize_t>(m_request.body().size()))
                m_done = true;

            Logger::trace("PostHandler: write %zu(%zu)", written, m_offset);
        }

        if (m_done)
        {
            Logger::trace("PostHandler: finished writing fd=%d", m_fd);

            // status
            m_response.set_status(StatusCode::Created);

            // body
            std::string json = "{"
                               "\"status\": \"success\","
                               "\"filename\": \"" +
                               m_upload.raw +
                               "\","
                               "\"size\": " +
                               utils::to_string(m_offset) + "}";
            m_response.set_body_as_str(json);

            // headers
            m_response.set_header("Location", m_upload.raw);
            m_response.set_header("Connection", "close"); // @NOTE: HTTP1.0 closes by default;
            m_response.set_header("Date", utils::http_date());
            m_response.set_header("Content-Type", "application/json");
            m_response.set_header("Content-Length", utils::to_string(json.size()));
        }
    }
    else
    {
        //@TODO: que código de erro é aqui? 404?
        http_utils::throw_internal_server_error_cant_upload(m_ctx);
    }
}

bool PostHandler::done() const
{
    return m_done;
}

const Response& PostHandler::response() const
{
    if (m_ctx.config().is_cgi())
        return m_cgi.response();
    return m_response;
}

PostHandler::~PostHandler()
{
    Logger::trace("PostHandler: destructor");
    if (m_fd > -1)
        close(m_fd);
};
