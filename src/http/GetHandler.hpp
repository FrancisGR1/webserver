#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "HttpResponse.hpp"
#include "AMethodHandler.hpp"

class GetHandler : public AMethodHandler 
{
	public:
		GetHandler();
		ssize_t send(int socket_fd) const;
		HttpResponse handle(const HttpRequest& request, const HttpRequestContext& ctx) const;
		~GetHandler();
};

#endif // GETHANDLER_HPP
