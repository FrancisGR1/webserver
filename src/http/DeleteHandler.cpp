#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "DeleteHandler.hpp"

DeleteHandler::DeleteHandler(const HttpRequest& request, const HttpRequestContext& ctx)
{
	(void) request;
	(void) ctx;
}

void DeleteHandler::process()
{
}

bool DeleteHandler::done() const
{
	return true;
}

const NewHttpResponse& DeleteHandler::response() const
{
	return m_response;
}

DeleteHandler::~DeleteHandler()
{
}
