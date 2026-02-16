#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "PostHandler.hpp"

PostHandler::PostHandler() {};

ssize_t PostHandler::send(int socket_fd) const
{
	(void) socket_fd;
	return 1;
}

HttpResponse PostHandler::handle(const HttpRequest& request, const HttpRequestContext& ctx) const
{
	return HttpResponse(request, ctx.config().service());
}

PostHandler::~PostHandler() {};
