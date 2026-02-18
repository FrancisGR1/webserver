#ifndef REQUESTDISPATCHER_HPP
# define REQUESTDISPATCHER_HPP

#include "core/Path.hpp"
#include "config/types/ServiceConfig.hpp"
#include "config/types/LocationConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "AMethodHandler.hpp"
#include "PostHandler.hpp"
#include "GetHandler.hpp"
#include "DeleteHandler.hpp"
#include "ErrorHandler.hpp"

class RequestDispatcher
{
	public:
		RequestDispatcher();
		HttpResponse dispatch(const HttpRequest& request, const ServiceConfig& service) const;

	private:
		PostHandler m_post_handler;
		GetHandler m_get_handler;
		DeleteHandler m_delete_handler;
		ErrorHandler m_error_handler;

		// helpers
		Path request_target_to_server_path(const HttpRequest& request, const ServiceConfig& service);
		HttpRequestContext make_context(const HttpRequest& request, const ServiceConfig& service);
		const AMethodHandler& select_handler(const HttpRequest& request) const;
		std::string resolved_target(const std::string& req_path, const ServiceConfig& service) const;
};

#endif // REQUESTDISPATCHER_HPP
