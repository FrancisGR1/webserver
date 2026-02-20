#include <vector>
#include <string>

#include "core/utils.hpp"
#include "core/Path.hpp"
#include "core/Logger.hpp"
#include "config/types/ServiceConfig.hpp"
#include "config/types/LocationConfig.hpp"
#include "StatusCode.hpp"
#include "HttpResponseException.hpp"
#include "AMethodHandler.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestConfig.hpp"
#include "HttpRequestContext.hpp"
#include "RequestDispatcher.hpp"
#include "PostHandler.hpp"
#include "GetHandler.hpp"
#include "DeleteHandler.hpp"
#include "ErrorHandler.hpp"


RequestDispatcher::RequestDispatcher() {}

AMethodHandler* RequestDispatcher::dispatch(const HttpRequest& request, const ServiceConfig& service) const
{
	try
	{
		// @NOTE: this code (request validation and route resolution) should not be responsibility of the dispatcher

		if (request.bad_request())
		{
			throw HttpResponseException(request.status_code(), "Bad request status code");
		}

		// get target path
		Path path = resolved_target(request.target_path(), service);
		if (!path.exists)
		{
			throw HttpResponseException(StatusCode::NotFound, utils::fmt("'%s' path not found", path.resolved.c_str()));
		}

		// @ASSUMPTION: if path exists, theres also a LocationConfig
		LocationConfig location = service.locations.at(path.resolved);
		HttpRequestConfig cfg(service, location, path);
		HttpRequestContext ctx(cfg); //@TODO: adicionar ligação, cookies, session manager

		// choose method
		if (request.method() == "GET")    return new GetHandler(request, ctx);
		if (request.method() == "POST")   return new PostHandler(request, ctx);
		if (request.method() == "DELETE") return new DeleteHandler(request, ctx);
		return new ErrorHandler(StatusCode::InternalServerError);
	}
	catch (const HttpResponseException& e)
	{
		//@TODO: return ErrorHandler(e);
		Logger::error("%s", e.msg().c_str());
	}
	catch (...)
	{
		//@TODO: return ErrorHandler(StatusCode::InternalServerError);
		Logger::error("Response: Server Error...");
	}
	return new ErrorHandler(StatusCode::InternalServerError);
}

std::string RequestDispatcher::resolved_target(const std::string& req_path, const ServiceConfig& service) const
{
	bool is_dir = !req_path.empty() &&
		req_path[req_path.size() - 1] == '/';

	// transform request path into clean path
	const std::vector<std::string>& segments = utils::str_split(req_path, "/");
	std::vector<std::string> legal_segments;
	for (size_t i = 0; i < segments.size(); ++i)
	{
		const std::string& s = segments[i];
		if (s.empty() || s == ".") 
			continue;
		if (s == "..")
		{
			if (!legal_segments.empty())
				legal_segments.pop_back();
			else // @NOTE: trying to escape root
			{
				throw HttpResponseException(
						StatusCode::BadRequest, 
						"Target path tried to escape root"
						);
			}
		}
		else
		{
			legal_segments.push_back(s);
		}
	}
	//@ASSUMPTION: target path always starts with '/'
	std::string cleaned_path = "/";
	for (size_t i = 0; i < legal_segments.size(); ++i)
	{
		cleaned_path += legal_segments[i];
		if (i + 1 < legal_segments.size() || is_dir)
			cleaned_path += "/";
	}

	// find the best matching location 
	std::string remainder;
	std::string location = cleaned_path;
	std::string resolved_path = cleaned_path;
	while (location != "/" && !location.empty())
	{

		size_t pos = location.find_last_of("/");
		location = location.substr(0, pos);
		remainder = cleaned_path.substr(pos);
		if (utils::contains(service.locations, location))
		{
			const std::string& root = service.locations.at(location).root_dir;
			resolved_path = utils::join_paths(root, remainder);
			break;
		}
	}

	return resolved_path;
}
