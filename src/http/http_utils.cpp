#include "core/utils.hpp"
#include "StatusCode.hpp"
#include "HttpResponseException.hpp"
#include "http_utils.hpp"

namespace http_utils
{
	// error throwers
	void throw_code(StatusCode::Code code, const HttpRequestContext& ctx) // generic
	{
		throw ResponseError(
				code, 
				"Bad request status code",
				ctx
				);
	}

	void throw_moved_permanently(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::MovedPermanently, 
				utils::fmt("%s was moved permanently", path.raw.c_str()),
				ctx
				);
	}

	void throw_bad_request_file(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::BadRequest,
				utils::fmt("'%s' is neither cgi nor a regular file", path.raw.c_str()),
				ctx
				);
	}

	void throw_bad_request_escape_root(const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::BadRequest, 
				"Target path tried to escape root",
				ctx
				);
	}

	void throw_method_not_allowed(const std::string& method, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::MethodNotAllowed,
				utils::fmt("%s not allowed", method.c_str()),
				ctx
				);
	}

	void throw_forbidden_invalid_route(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::Forbidden, 
				utils::fmt("Invalid route: %s", path.raw.c_str()),
				ctx
				);
	}

	void throw_forbidden_invalid_directory(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::Forbidden, 
				utils::fmt("Invalid directory: %s", path.raw.c_str()),
				ctx
				);
	}

	void throw_forbidden_cant_access_directory(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::Forbidden, 
				utils::fmt("Can't access directory: '%s'" , path.raw.c_str()),
				ctx
				);
	}

	void throw_forbidden_cant_read_file(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("Can't read file: '%s'", path.raw.c_str()),
				ctx
				);
	}

	void throw_forbidden_not_regular_file(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("Not a regular file: '%s'", path.raw.c_str()),
				ctx
				);
	}

	void throw_forbidden_cant_do_anything_with_directory(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("Directory '%s' has no index and autoindex", path.raw.c_str()),
				ctx
				);
	}

	void throw_forbidden_cant_upload(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("'%s' upload directory doesn't have write and/or execution permission(s)", path.raw.c_str()),
				ctx
				);
	}

	void throw_not_found(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::NotFound,
				utils::fmt("'%s' path not found", path.raw.c_str()),
				ctx
				);
	}

	void throw_conflict(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::Conflict,
				utils::fmt("Cannot delete directory: '%s", path.raw.c_str()),
				ctx
				);
	}

	void throw_content_too_large(const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::ContentTooLarge,
				utils::fmt("Request body size is larger than expected"),
				ctx
				);
	}

	void throw_internal_server_error_cant_delete(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Failed to delete file: '%s", path.raw.c_str()),
				ctx
				);
	}

	void throw_internal_server_error_not_valid(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Not valid: '%s'", path.raw.c_str()),
				ctx
				);
	}

	void throw_internal_server_error_cant_upload(const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Can't upload file: location is missing upload directory path"),
				ctx
				);
	}

	void throw_internal_server_error_doesnt_exist(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("'%s' doesn't exist", path.raw.c_str()),
				ctx
				);
	}

	void throw_internal_server_error_not_a_directory(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("'%s' is not a directory", path.raw.c_str()),
				ctx
				);
	}

	void throw_internal_server_error_failed_upload(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Failed when creating uploaded file: '%s'", path.raw.c_str()),
				ctx
				);
	}

	void throw_internal_server_error_unknown_file_type(const Path& path, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Uknown file type '%s", path.raw.c_str()),
				ctx
				);
	}

	void throw_not_implemented(const std::string& method, const HttpRequestContext& ctx)
	{
		throw ResponseError(
				StatusCode::NotImplemented, 
				utils::fmt("%s method is not implemented by the server", method.c_str()),
				ctx
				);
	}


	static std::string RequestDispatcher::resolve_target(const std::string& req_path, const ServiceConfig& service) const
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
					throw ResponseError(
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

	// given a request and a service config, sets up:
	// - resolved path
	// - location config, if exists
	// - context class
	// @NOTE: if it can't at least set a path, throws exception
	void resolve_request(HttpRequest& request, const ServiceConfig& service, HttpRequestConfig& config, HttpRequestContext& context)
	{
		if (request.bad_request())
		{
			throw ResponseError(request.status_code(), "Bad request status code");
		}

		// transform request path to resolved path
		Path path = resolve_target(request.target_path(), service);

		// validate and set path in config
		if (!path.exists)
			throw ResponseError(StatusCode::NotFound, utils::fmt("'%s' path not found", path.raw.c_str()));
		config.set_path(path);

		// validate and set location in config
		if (utils::contains(service.locations, path.raw))
		{
			config.set_location(path);
		}

		// add config to context
		context.set_config(config);
	}

} // namespace http_utils
