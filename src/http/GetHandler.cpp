#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "GetHandler.hpp"

GetHandler::GetHandler(const HttpRequest& request, const HttpRequestContext& ctx)
{
	(void) request;
	(void) ctx;
}

void GetHandler::process()
{
}

bool GetHandler::done() const
{
	return true;
}

const NewHttpResponse& GetHandler::response() const
{
	return m_response;
}

GetHandler::~GetHandler()
{
}
