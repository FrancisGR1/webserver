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
		AMethodHandler* dispatch(const HttpRequest& request, const ServiceConfig& service) const;

	private:

		// helpers
		std::string resolved_target(const std::string& req_path, const ServiceConfig& service) const;
};

#endif // REQUESTDISPATCHER_HPP
