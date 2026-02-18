#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "GetHandler.hpp"

GetHandler::GetHandler() {};

ssize_t GetHandler::send(int socket_fd) const
{
	(void) socket_fd;
	return 1;
}

HttpResponse GetHandler::handle(const HttpRequest& request, const HttpRequestContext& ctx) const
{
	return HttpResponse(request, ctx.config().service());
}

GetHandler::~GetHandler() {};
