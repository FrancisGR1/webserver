#include <cstdio>

#include "DeleteHandler.hpp"
#include "core/Logger.hpp"
#include "core/utils.hpp"
#include "http/http_utils.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/request/Request.hpp"

DeleteHandler::DeleteHandler(const Request& request, const RequestContext& ctx)
    : m_ctx(ctx)
    , m_done(false)
    , m_cgi(request, ctx, constants::cgi_timeout)
{
    Logger::trace("DeleteHandler: constructor");
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
    else if (config.is_cgi())
    {
        m_cgi.process();
        if (m_cgi.done())
            m_done = true;
        return;
    }

    const Path& path = config.path();

    if (path.is_directory)
        http_utils::throw_conflict(path, m_ctx);

    // delete file
    if (std::remove(path.raw.c_str()) != 0)
    {
        http_utils::throw_internal_server_error_cant_delete(path, m_ctx);
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
    if (m_ctx.config().is_cgi())
        return m_cgi.response();
    return m_response;
}

DeleteHandler::~DeleteHandler()
{
    Logger::trace("DeleteHandler: destructor");
}
