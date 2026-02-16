#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "HttpResponse.hpp"
#include "AMethodHandler.hpp"

class PostHandler : public AMethodHandler 
{
	//type - POST, GET, DELETE
	public:
		PostHandler();
		ssize_t send(int socket_fd) const;
		HttpResponse handle(const HttpRequest& request, const HttpRequestContext& ctx) const;
		~PostHandler();
};

#endif // POSTHANDLER_HPP
