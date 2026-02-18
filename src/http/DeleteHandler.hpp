#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "HttpResponse.hpp"
#include "AMethodHandler.hpp"

class DeleteHandler : public AMethodHandler 
{
	public:
		DeleteHandler();
		ssize_t send(int socket_fd) const;
		HttpResponse handle(const HttpRequest& request, const HttpRequestContext& ctx) const;
		~DeleteHandler();
};

#endif // DELETEHANDLER_HPP
