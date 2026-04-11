#include "ErrorHandler.hpp"
#include "core/Logger.hpp"
#include "core/MimeTypes.hpp"
#include "core/utils.hpp"
#include "http/processor/RequestContext.hpp"

ErrorHandler::ErrorHandler(const ResponseError& error)
    : m_code(error.status_code())
    , m_ctx(error.has_ctx() ? &error.ctx() : NULL)
    , m_done(false)
{
    Logger::trace("ErrorHandler: constructor");
}

ErrorHandler::ErrorHandler(StatusCode::Code code)
    : m_code(code)
    , m_ctx(NULL)
    , m_done(false)
{
    Logger::trace("ErrorHandler: constructor");
}

ErrorHandler::ErrorHandler(StatusCode::Code code, const RequestContext& ctx)
    : m_code(code)
    , m_ctx(&ctx)
    , m_done(false)
{
    Logger::trace("ErrorHandler: constructor");
}

// all in one go
void ErrorHandler::process()
{
    Logger::trace("ErrorHandler: processing...");

    // status
    m_response.set_status(m_code);

    // default headers
    m_response.set_header("Connection", "close");
    m_response.set_header("Connection", "close");
    m_response.set_header("Date", utils::http_date());

    if (m_ctx != NULL)
    {
        const RequestConfig& config = m_ctx->config();
        const Path& error_page = config.get_error_page_or_nonexistent_path(m_code);
        if (error_page.exists)
        {
            // headers
            m_response.set_header("Content-Length", utils::to_string(error_page.size));
            m_response.set_header("Content-Type", error_page.mime);

            // body
            m_response.set_body_as_path(error_page);

            m_done = true;

            return;
        }
    }

    std::string html = make_default_body(m_code);

    // headers
    m_response.set_header("Content-Length", utils::to_string(html.size()));
    m_response.set_header("Content-Type", MimeTypes::from_extension("html"));

    // body
    m_response.set_body_as_str(html);

    m_done = true;
}

bool ErrorHandler::done() const
{
    return m_done;
}

const Response& ErrorHandler::response() const
{
    return m_response;
}

std::string ErrorHandler::make_default_body(StatusCode::Code code)
{
    // build default body
    size_t error = static_cast<size_t>(code);
    std::string title = utils::to_string(error) + " " + StatusCode::to_reason(code);
    std::string html = "<html>\n"
                       "<head><title>" +
                       title +
                       "</title></head>\n"
                       "<body>\n"
                       "<center><h1>" +
                       title +
                       "</h1></center>\n"
                       "</body>\n"
                       "</html>\n";
    return html;
}

ErrorHandler::~ErrorHandler()
{
    Logger::trace("ErrorHandler: destructor");
}
