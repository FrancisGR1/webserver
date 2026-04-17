#include "http/http_utils.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "response/ResponseError.hpp"

// clang-format off
namespace http_utils
{
	// error throwers
	void throw_code(StatusCode::Code code, const RequestContext& ctx) // generic
	{
	    throw ResponseError(code, 
			    "Bad request status code", &ctx);
	}

	void throw_moved_permanently(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::MovedPermanently, 
			    utils::fmt("%s was moved permanently", path.raw.c_str()), &ctx);
	}
	
	void throw_bad_request_file(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::BadRequest, 
			    utils::fmt("'%s' is neither cgi nor a regular file", path.raw.c_str()), &ctx);
	}
	
	void throw_bad_request_escape_root(const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::BadRequest, 
			    "Target path tried to escape root", &ctx);
	}
	
	void throw_method_not_allowed(const std::string& method, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::MethodNotAllowed, 
			    utils::fmt("%s not allowed", method.c_str()), &ctx);
	}
	
	void throw_forbidden_invalid_route(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Forbidden, 
			    utils::fmt("Invalid route: %s", path.raw.c_str()), &ctx);
	}
	
	void throw_forbidden_invalid_directory(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Forbidden, 
			    utils::fmt("Invalid directory: %s", path.raw.c_str()), &ctx);
	}
	
	void throw_forbidden_cant_access_directory(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Forbidden, 
			    utils::fmt("Can't access directory: '%s'", path.raw.c_str()), &ctx);
	}
	
	void throw_forbidden_cant_read_file(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Forbidden, 
			    utils::fmt("Can't read file: '%s'", path.raw.c_str()), &ctx);
	}
	
	void throw_forbidden_not_regular_file(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Forbidden, 
			    utils::fmt("Not a regular file: '%s'", path.raw.c_str()), &ctx);
	}
	
	void throw_forbidden_cant_do_anything_with_directory(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Forbidden, 
			    utils::fmt("Directory '%s' has no index and autoindex", path.raw.c_str()), &ctx);
	}
	
	void throw_forbidden_cant_upload(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Forbidden, 
			    utils::fmt("'%s' upload directory doesn't have permission(s)", path.raw.c_str()), &ctx);
	}
	
	void throw_not_found(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::NotFound, 
			    utils::fmt("'%s' path not found", path.raw.c_str()), &ctx);
	}
	
	void throw_conflict_delete(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Conflict, 
			    utils::fmt("Cannot delete directory: '%s'", path.raw.c_str()), &ctx);
	}

	void throw_conflict_upload(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::Conflict, 
			    utils::fmt("Trying to upload to a non-directory: '%s", path.raw.c_str()), &ctx);
	}
	
	void throw_content_too_large(const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::ContentTooLarge, 
			    utils::fmt("Request body size is larger than expected"), &ctx);
	}
	
	void throw_internal_server_error_cant_delete(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::InternalServerError, 
			    utils::fmt("Failed to delete file: '%s", path.raw.c_str()), &ctx);
	}
	
	void throw_internal_server_error_not_valid(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::InternalServerError, 
			    utils::fmt("Not valid: '%s'", path.raw.c_str()), &ctx);
	}
	
	void throw_internal_server_error_cant_upload(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::InternalServerError, 
			    utils::fmt("Can't upload file: location is missing upload directory path for: '%s'", path.raw.c_str()), &ctx);
	}
	
	void throw_internal_server_error_doesnt_exist(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::InternalServerError, 
			    utils::fmt("'%s' doesn't exist", path.raw.c_str()), &ctx);
	}
	
	void throw_internal_server_error_not_a_directory(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::InternalServerError, 
			    utils::fmt("'%s' is not a directory", path.raw.c_str()), &ctx);
	}
	
	void throw_internal_server_error_failed_upload(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::InternalServerError, 
			    utils::fmt("Failed when creating uploaded file: '%s'", path.raw.c_str()), &ctx);
	}
	
	void throw_internal_server_error_unknown_file_type(const Path& path, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::InternalServerError, 
			    utils::fmt("Unknown file type '%s", path.raw.c_str()), &ctx);
	}
	
	void throw_not_implemented(const std::string& method, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::NotImplemented, 
			    utils::fmt("%s method is not implemented by the server", method.c_str()), &ctx);
	}
	
	void throw_gateway_timeout(const std::string& cgi_raw_path, Seconds timeout, const RequestContext& ctx)
	{
	    throw ResponseError(StatusCode::GatewayTimeout, 
			    utils::fmt( "Timeout: CGI script '%s' took more than %.2f seconds", cgi_raw_path.c_str(), timeout), &ctx);
	}

} // namespace http_util&s
// clang-format on
