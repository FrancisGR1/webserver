#include <ctime>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "GetHandler.hpp"
#include "core/Logger.hpp"
#include "core/MimeTypes.hpp"
#include "core/utils.hpp"
#include "http/http_utils.hpp"
#include "http/processor/RequestContext.hpp"

GetHandler::GetHandler(const RequestContext& ctx)
    : m_ctx(ctx)
    , m_done(false)
    , m_get_fd(-1)
{
    Logger::trace("GetHandler: constructor");
}

GetHandler::~GetHandler()
{
    if (m_get_fd > -1)
    {
        Logger::trace("GetHandler: close '%d'", m_get_fd);
        ::close(m_get_fd);
    }
    Logger::trace("GetHandler: destructor");
}

//@TODO implementar If-Modified-Since e If-None-Match (ETag), Range requests
void GetHandler::process()
{
    Logger::trace("GetHandler: processing...");

    const RequestConfig& config = m_ctx.config();

    if (!config.allows_method("GET"))
        http_utils::throw_method_not_allowed("GET", m_ctx);

    if (config.is_redirected())
    {
        m_response.make_redirection_response(StatusCode::MovedPermanently, config.redirection());
        m_done = true;
        return;
    }

    const Path& path = config.path();
    if (path.is_directory)
    {

        if (!path.can_execute)
            http_utils::throw_forbidden_cant_access_directory(path, m_ctx);
        //@TODO deve redirecionar para o novo url

        if (config.has_index())
        {
            // index path
            // join index file to location root directive (if existent)
            const Path index_path = utils::join_paths(config.root().raw, config.index().raw);
            if (!index_path.exists)
                http_utils::throw_not_found(index_path, m_ctx);
            if (!index_path.is_regular_file)
                http_utils::throw_forbidden_not_regular_file(index_path, m_ctx);
            if (!index_path.can_read)
                http_utils::throw_forbidden_cant_read_file(index_path, m_ctx);

            // index path is valid
            handle_index(m_response, index_path);
        }
        else if (config.allows_autoindex())
        {
            handle_autoindex(m_response, path);
        }
        else
        {
            http_utils::throw_forbidden_cant_do_anything_with_directory(path, m_ctx);
        }
    }
    else
    {
        if (!path.exists)
            http_utils::throw_not_found(path, m_ctx);
        if (!path.is_regular_file || !path.can_read)
            http_utils::throw_forbidden_cant_read_file(path, m_ctx);

        // ok
        handle_file(m_response, path);
    }

    m_done = true;
}

void GetHandler::handle_index(Response& response, const Path& path)
{
    Logger::trace("GetHandler: make index response");
    // status
    response.set_status(StatusCode::Ok);
    // headers
    response.set_header("Connection", "close"); // @NOTE: HTTP-1.0 closes by default
    response.set_header("Content-Length", utils::to_string(path.size));
    response.set_header("Content-Type", path.mime);
    response.set_header("Date", utils::http_date());
    // body
    m_get_fd = response.set_body_as_path(path);
}

std::string GetHandler::make_autoindex(const Path& path)
{
    DIR* dir = opendir(path.raw.c_str());
    if (!dir)
    {
        http_utils::throw_forbidden_invalid_directory(path, m_ctx);
    }

    std::string body = "<html>\n"
                       "<head><title>Index of " +
                       path.raw +
                       "</title></head>\n"
                       "<body>\n"
                       "<h1>Index of " +
                       path.raw + "</h1><hr><pre>\n";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        //@TODO: .. pode passar se não for root
        if (name == "." || name == "..")
            continue;

        std::string full_path = path.raw + "/" + name;

        struct stat st;
        if (stat(full_path.c_str(), &st) == 1)
            continue;

        bool is_dir = S_ISDIR(st.st_mode);

        // link
        body += "<a href=\"";
        body += name;
        if (is_dir)
            body += "/";
        body += "\">";
        body += name;
        if (is_dir)
            body += "/";
        body += "</a>";

        // spacing
        size_t pad = 49;
        if (name.length() < pad)
            body += std::string(pad - name.length(), ' ');
        size_t display_len = name.length() + (is_dir ? 1 : 0);
        if (display_len < pad)
            body += std::string(pad - display_len, ' ');

        // date
        char timebuf[31];
        std::tm* tm = std::localtime(&st.st_mtime);
        std::strftime(timebuf, sizeof(timebuf), "%d-%b-%Y %H:%M", tm);
        body += timebuf;
        body += " ";

        // size
        if (is_dir)
            body += "                  -";
        else
        {
            std::string file_size = utils::to_string(st.st_size);
            body += std::string(17 - file_size.size(), ' ') + file_size;
        }
        body += "\n";
    }

    closedir(dir);

    body += "</pre><hr></body>\n</html>\n";

    return body;
}

void GetHandler::handle_autoindex(Response& response, const Path& path)
{
    Logger::trace("GetHandler: make autoindex response");
    // status
    response.set_status(StatusCode::Ok);
    // header
    response.set_header("Connection", "close"); // @NOTE: HTTP-1.0 closes by default
    response.set_header("Content-Length", utils::to_string(path.size));
    response.set_header("Content-Type", MimeTypes::from_extension("html"));
    response.set_header("Date", utils::http_date());
    // body
    std::string autoindex = make_autoindex(path);
    response.set_body_as_str(autoindex);
}

void GetHandler::handle_file(Response& response, const Path& path)
{
    Logger::trace("GetHandler: get file: '%s'", path.raw.c_str());
    // status
    response.set_status(StatusCode::Ok);
    // header
    response.set_header("Connection", "close"); // @NOTE: HTTP-1.0 closes by default
    response.set_header("Content-Length", utils::to_string(path.size));
    response.set_header("Content-Type", path.mime);
    response.set_header("Date", utils::http_date());
    //@QUESTION: mais headers? Last-Modified? ETag?
    // body
    m_get_fd = response.set_body_as_path(path);
}

bool GetHandler::done() const
{
    return m_done;
}

const Response& GetHandler::response() const
{
    return m_response;
}

std::vector<EventAction> GetHandler::give_events()
{
    return std::vector<EventAction>();
}
