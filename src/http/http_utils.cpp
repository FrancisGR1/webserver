#include "StatusCode.hpp"
#include "HttpResponseException.hpp"
#include "http_utils.hpp"

namespace http_utils
{
	// error throwers
	void throw_code(StatusCode::Code code) // generic
	{
		throw ResponseError(
				code, 
				"Bad request status code"
				);
	}

	void throw_moved_permanently(const Path& path)
	{
		throw ResponseError(
				StatusCode::MovedPermanently, 
				utils::fmt("%s was moved permanently", path.resolved.c_str())
				);
	}

	void throw_bad_request_file(const Path& path)
	{
		throw ResponseError(
				StatusCode::BadRequest,
				utils::fmt("'%s' is neither cgi nor a regular file", path.resolved.c_str()),
				);
	}

	void throw_bad_request_escape_root()
	{
		throw ResponseError(
				StatusCode::BadRequest, 
				"Target path tried to escape root"
				);
	}

	void throw_method_not_allowed(const LocationConfig& location, const std::string& method)
	{
		throw ResponseError(
				StatusCode::MethodNotAllowed,
				utils::fmt("%s not allowed on this location: '%s'", method.c_str(), location.name.c_str())
				);
	}

	void throw_forbidden_invalid_route(const Path& path)
	{
		throw ResponseError(
				StatusCode::Forbidden, 
				utils::fmt("Invalid route: %s", path.resolved.c_str())
				);
	}

	void throw_forbidden_invalid_directory(const Path& path)
	{
		throw ResponseError(
				StatusCode::Forbidden, 
				utils::fmt("Invalid directory: %s", path.resolved.c_str())
				);
	}

	void throw_forbidden_cant_access_directory(const Path& path)
	{
		throw ResponseError(
				StatusCode::Forbidden, 
				utils::fmt("Can't access directory: '%s'" , path.resolved.c_str()), 
				);
	}

	void throw_forbidden_cant_read_file(const Path& path)
	{
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("Can't read file: '%s'", path.resolved.c_str())
				);
	}

	void throw_forbidden_not_regular_file(const Path& path)
	{
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("Not a regular file: '%s'", path.resolved.c_str()),
				);
	}

	void throw_forbidden_cant_do_anything_with_directory(const Path& path)
	{
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("Directory '%s' has no index and autoindex", path.resolved.c_str()),
				);
	}

	void throw_forbidden_cant_upload(const Path& path)
	{
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("'%s' upload directory doesn't have write and/or execution permission(s)", path.resolved.c_str())
				);
	}

	void throw_not_found(const Path& path)
	{
		throw ResponseError(
				StatusCode::NotFound,
				utils::fmt("'%s' path not found", path.resolved.c_str()),
				);
	}

	void throw_conflict(const Path& path)
	{
		throw ResponseError(
				StatusCode::Conflict,
				utils::fmt("Cannot delete directory: '%s", path.resolved.c_str()),
				);
	}

	void throw_content_too_large(const Path& path)
	{
		throw ResponseError(
				StatusCode::ContentTooLarge,
				utils::fmt("Request body size is larger than expected"),
				);
	}

	void throw_internal_server_error_cant_delete(const Path& path)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Failed to delete file: '%s", path.resolved.c_str())
				);
	}

	void throw_internal_server_error_not_valid(const Path& path)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Not valid: '%s'", path.resolved.c_str())
				);
	}

	void throw_internal_server_error_cant_upload(const Path& path)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Can't upload file: location '%s' is missing upload directory path", lc.name.c_str())
				);
	}

	void throw_internal_server_error_doesnt_exist(const Path& path)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("'%s' doesn't exist", upload_dir.resolved.c_str())
				);
	}

	void throw_internal_server_error_not_a_directory(const Path& path)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("'%s' is not a directory", upload_dir.resolved.c_str())
				);
	}

	void throw_internal_server_error_failed_upload(const Path& path)
	{
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Failed when creating uploaded file: '%s'", upload_path.c_str())
				);
	}

	void throw_not_implemented(const HttpRequest& request)
	{
		throw ResponseError(
				StatusCode::NotImplemented, 
				utils::fmt("%s method is not implemented by the server", request.method().c_str())
				);
	}

} // namespace http_utils
