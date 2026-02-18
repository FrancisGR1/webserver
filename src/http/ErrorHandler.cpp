#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler() {};

ssize_t ErrorHandler::send(int socket_fd) const
{
	(void) socket_fd;
	return 1;
}

HttpResponse ErrorHandler::handle(const HttpRequest& request, const HttpRequestContext& ctx) const
{
	return HttpResponse(request, ctx.config().service());
}

ErrorHandler::~ErrorHandler() {};
