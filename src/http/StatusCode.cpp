#include "StatusCode.hpp"

std::string StatusCode::to_reason(StatusCode::Code c)
{
	typedef StatusCode SC;

	switch (c)
	{
		// === Successful ===
		case SC::Ok       : return "OK";
		case SC::Created  : return "Created";
		case SC::Accepted : return "Accepted";
		case SC::NoContent: return "No Content";

		// === Redirection ===
		case SC::MovedPermanently : return "Moved Permanently";
		case SC::PermanentRedirect: return "Permanent Redirect";
		
		// === Client Side Error ===
		case SC::BadRequest                 : return "Bad Request";
		case SC::Forbidden                  : return "Forbidden";
		case SC::NotFound                   : return "Not Found";
		case SC::MethodNotAllowed           : return "Method Not Allowed";
		case SC::NotAcceptable              : return "Not Acceptable";
		case SC::Conflict                   : return "Conflict";
		case SC::LengthRequired             : return "Length Required";
		case SC::ContentTooLarge            : return "Content Too Large";
		case SC::UriTooLong                 : return "URI Too Long";
		case SC::UnsupportedMediaType       : return "Unsupported Media Type";
		case SC::TooManyRequests            : return "Too Many Requests";
		case SC::RequestHeaderFieldsTooLarge: return "Request Header Fields Too Large";

		// === Server Side Error ===
		case SC::InternalServerError    : return "Internal Server Error";
		case SC::NotImplemented         : return "Not Implemented";
		case SC::BadGateway             : return "Bad Gateway";
		case SC::ServiceUnavailable     : return "Service Unavailable";
		case SC::GatewayTimeout         : return "Gateway Timeout";
		case SC::HttpVersionNotSupported: return "HTTP Version Not Supported";

		// @NOTE: should never reach here
		default : return "OK";
	};
}

bool StatusCode::is_redirection(size_t code)
{
	return (
		code == StatusCode::MovedPermanently  ||
		code == StatusCode::Found             ||
		code == StatusCode::SeeOther          ||
		code == StatusCode::TemporaryRedirect ||
		code == StatusCode::PermanentRedirect
	       );
}
