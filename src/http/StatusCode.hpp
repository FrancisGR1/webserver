#ifndef STATUSCODE_HPP
#define STATUSCODE_HPP

#include <string>

// @TODO mudar para classe
struct StatusCode
{

	// https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status
	enum Code
	{
		None = 0, 
		// === Informational ===

		// === Successful ===
		Ok        = 200,
		Created   = 201,
		Accepted  = 202,
		NoContent = 204,

		// === Redirection ===
		MovedPermanently  = 301,
		Found  		  = 302,
		SeeOther          = 303,
		TemporaryRedirect = 307,
		PermanentRedirect = 308,
		
		// === Client Side Error ===
		BadRequest                  = 400,
		Forbidden                   = 403,
		NotFound                    = 404,
		MethodNotAllowed            = 405,
		NotAcceptable               = 406,
		Conflict                    = 409,
		LengthRequired              = 411,
		ContentTooLarge             = 413,
		UriTooLong                  = 414,
		UnsupportedMediaType        = 415,
		TooManyRequests             = 429,
		RequestHeaderFieldsTooLarge = 431,

		// === Server Side Error ===
		InternalServerError     = 500,
		NotImplemented          = 501,
		BadGateway              = 502,
		ServiceUnavailable      = 503,
		GatewayTimeout          = 504,
		HttpVersionNotSupported = 505,
	};

	static std::string to_reason(StatusCode::Code c);
	static bool is_redirection(size_t code);
	//@TODO:
	//is_valid(size_t code)
	//str()
};

#endif //STATUSCODE_HPP
