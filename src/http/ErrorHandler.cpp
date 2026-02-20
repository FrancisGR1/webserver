#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler(StatusCode::Code code)
{
	(void) code;
}

void ErrorHandler::process()
{
}

bool ErrorHandler::done() const
{
	return true;
}

const NewHttpResponse& ErrorHandler::response() const
{
	return m_response;
}

ErrorHandler::~ErrorHandler()
{
}
