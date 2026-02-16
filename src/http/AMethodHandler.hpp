#ifndef AMETHODHANDLER_HPP
#define AMETHODHANDLER_HPP

#include <string>

#include "config/ConfigTypes.hpp"
#include "HttpRequestContext.hpp"
#include "HttpResponse.hpp"

class AMethodHandler
{
	public:
		//type - POST, GET, DELETE
		virtual ssize_t send(int socket_fd) const = 0;
		virtual HttpResponse handle(const HttpRequest& request, const HttpRequestContext& ctx) const = 0;
		virtual ~AMethodHandler() = 0;
};

#endif // AMETHODHANDLER_HPP
