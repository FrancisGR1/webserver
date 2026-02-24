#ifndef HTTPUTILS_HPP
# define HTTPUTILS_HPP

#include <string>

#include "http/StatusCode.hpp"
#include "core/Path.hpp"
#include "processor/RequestContext.hpp"

namespace http_utils
{
	//@TODO: give ctx to every one of these
	//@TODO: also status code doesnt have to be said in function
	// error throwers
	// generic
	void throw_code(StatusCode::Code code, const RequestContext& ctx);
	// MovedPermanently 301
	void throw_moved_permanently(const Path& path, const RequestContext& ctx);
	// BadRequest 400
	void throw_bad_request_file(const Path& path, const RequestContext& ctx);
	void throw_bad_request_escape_root(const RequestContext& ctx);
	// MethodNotAllowed 405
	void throw_method_not_allowed(const std::string& method, const RequestContext& ctx);
	// Forbidden 403
	void throw_forbidden_invalid_route(const Path& path, const RequestContext& ctx);
	void throw_forbidden_invalid_directory(const Path& path, const RequestContext& ctx);
	void throw_forbidden_cant_access_directory(const Path& path, const RequestContext& ctx);
	void throw_forbidden_cant_read_file(const Path& path, const RequestContext& ctx);
	void throw_forbidden_not_regular_file(const Path& path, const RequestContext& ctx);
	void throw_forbidden_cant_do_anything_with_directory(const Path& path, const RequestContext& ctx);
	void throw_forbidden_cant_upload(const Path& path, const RequestContext& ctx);
	// NotFound 404
	void throw_not_found(const Path& path, const RequestContext& ctx);
	// Conflict 409
	void throw_conflict(const Path& path, const RequestContext& ctx);
	// ContentTooLarge 413
	void throw_content_too_large(const RequestContext& ctx);
	// InternalServerError 500
	void throw_internal_server_error_cant_delete(const Path& path, const RequestContext& ctx);
	void throw_internal_server_error_not_valid(const Path& path, const RequestContext& ctx);
	void throw_internal_server_error_cant_upload(const RequestContext& ctx);
	void throw_internal_server_error_doesnt_exist(const Path& path, const RequestContext& ctx);
	void throw_internal_server_error_not_a_directory(const Path& path, const RequestContext& ctx);
	void throw_internal_server_error_failed_upload(const Path& path, const RequestContext& ctx);
	void throw_internal_server_error_unknown_file_type(const Path& path, const RequestContext& ctx);
	// NotImplemented 501
	void throw_not_implemented(const std::string& method, const RequestContext& ctx);
	// GatewayTimeout 504
	void throw_gateway_timeout(const std::string& cgi_raw_path, const RequestContext& ctx);
} // namespace http_utils

#endif // HTTPUTILS_HPP
