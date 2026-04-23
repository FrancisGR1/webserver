#include <cstdio>
#include <cstring>

#include "core/Logger.hpp"
#include "core/ResourceLocker.hpp"
#include "core/utils.hpp"
#include "http/http_utils.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/DeleteHandler.hpp"

DeleteHandler::DeleteHandler(const RequestContext& ctx)
    : m_ctx(ctx)
    , m_done(false)
{
    Logger::trace("DeleteHandler: constructor");
}

DeleteHandler::~DeleteHandler()
{
    Logger::trace("DeleteHandler: destructor");
}

void DeleteHandler::process()
{
    const RequestConfig& config = m_ctx.config();

    if (!config.allows_method("DELETE"))
        http_utils::throw_method_not_allowed("DELETE", m_ctx);
    if (config.is_redirected())
    {
        m_response.make_redirection_response(StatusCode::MovedPermanently, config.redirection());
        m_done = true;
    }

    const Path& path = config.path();

    if (path.is_directory)
        http_utils::throw_conflict_delete(path, m_ctx);

    if (!ResourceLocker::lock(path))
        http_utils::throw_service_unavailable(path, m_ctx);

    // delete file
    if (std::remove(path.raw.c_str()) != 0)
    {
        int err = errno;
        Logger::error("Remove failed: errno says: '%s'", strerror(errno));
        switch (err)
        {
            case EACCES:
            case EPERM: http_utils::throw_forbidden_invalid_directory(path, m_ctx); break;
            case ENOENT: http_utils::throw_not_found(path, m_ctx); break;
            case EISDIR: http_utils::throw_conflict_delete(path, m_ctx); break;
            default: http_utils::throw_internal_server_error_cant_delete(path, m_ctx); break;
        }
    }

    // status
    m_response.set_status(StatusCode::NoContent);
    // headers
    m_response.set_header("Connection", "close"); // @NOTE: HTTP1.0 closes by default
    m_response.set_header("Date", utils::http_date());

    m_done = true;
}

bool DeleteHandler::done() const
{
    return m_done;
}

const Response& DeleteHandler::response() const
{
    return m_response;
}

std::vector<EventAction> DeleteHandler::give_events()
{
    return std::vector<EventAction>();
}
