#include "core/MimeTypes.hpp"
#include "config/ConfigTypes.hpp"
#include "http/processor/RequestContext.hpp"
#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler(const ResponseError& error)
	: m_code(error.status())
	, m_ctx(error.has_ctx() ? &error.ctx() : NULL)
	, m_done(false) {}

ErrorHandler::ErrorHandler(StatusCode::Code code)
	: m_code(code)
	, m_ctx(NULL)
	, m_done(false) {}

ErrorHandler::ErrorHandler(StatusCode::Code code, const RequestContext& ctx)
	: m_code(code)
	, m_ctx(&ctx)
	, m_done(false) {}

// all in one go
void ErrorHandler::process()
{
	size_t error = static_cast<size_t>(m_code);

	// status
	m_response.set_status(m_code);

	// default headers
	m_response.set_header("Connection", "close");
	m_response.set_header("Connection", "close");
	m_response.set_header("Date", utils::http_date());

	if (m_ctx != NULL)
	{
		const RequestConfig& config = m_ctx->config();
		const Path& error_page = config.get_error_page_or_nonexistent_path(error);
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


	// build default body
	std::string title = utils::to_string(error) + " " + 
		StatusCode::to_reason(m_code);

	std::string html = \
			   "<html>\n" 
			   "<head><title>" + title + "</title></head>\n" 
			   "<body>\n" 
			   "<center><h-1>"  + title + "</h1></center>\n" 
			   "</body>\n" 
			   "</html>\n";

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

ErrorHandler::~ErrorHandler()
{
	delete m_ctx;
}
