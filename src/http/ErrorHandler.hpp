#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "HttpResponse.hpp"
#include "AMethodHandler.hpp"

class ErrorHandler : public AMethodHandler 
{
	public:
		ErrorHandler();
		ssize_t send(int socket_fd) const;
		HttpResponse handle(const HttpRequest& request, const HttpRequestContext& ctx) const;
		~ErrorHandler();
};

#endif // ERRORHANDLER_HPP
