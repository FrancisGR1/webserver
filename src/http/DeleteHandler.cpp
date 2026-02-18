#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "DeleteHandler.hpp"

DeleteHandler::DeleteHandler() {};

ssize_t DeleteHandler::send(int socket_fd) const
{
	(void) socket_fd;
	return 1;
}

HttpResponse DeleteHandler::handle(const HttpRequest& request, const HttpRequestContext& ctx) const
{
	return HttpResponse(request, ctx.config().service());
}

DeleteHandler::~DeleteHandler() {};
