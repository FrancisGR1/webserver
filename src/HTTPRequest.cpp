#include <string>
#include <ostream>
#include <map>
#include <iomanip>
#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() {}
HTTPRequest::HTTPRequest(const std::string& method,
			const std::string& target_path,
			const std::string& target_query,
			const std::string& protocol_version,
			const std::map<std::string, std::string>& headers,
			const std::string& body)
	: m_method(method)
	, m_target_path(target_path)
	, m_target_query(target_query)
	, m_protocol_version(protocol_version)
	, m_headers(headers)
	, m_body(body)
{}

const std::string& HTTPRequest::method() const { return m_method; }
const std::string& HTTPRequest::target_path() const { return m_target_path; }
const std::string& HTTPRequest::target_query() const { return m_target_query; }
const std::string& HTTPRequest::protocol_version() const { return m_protocol_version; }
const std::map<std::string, std::string>& HTTPRequest::headers() const { return m_headers; }
const std::string& HTTPRequest::body() const { return m_body; }

static void print_field(std::ostream& os, const std::string& title, const std::string& value, int width = 14)
{
    os << std::left << std::setw(width) << (title + ":") << value << "\n";
}

std::ostream& operator<<(std::ostream& os, const HTTPRequest& request)
{
    os << "\n";
    print_field(os, "Method",  request.method());
    print_field(os, "Target Path",  request.target_path());
    print_field(os, "Target Query",  request.target_query());
    print_field(os, "Version", request.protocol_version());

    os << std::left << std::setw(10) << "Headers:" << "\n";
    for (std::map<std::string,std::string>::const_iterator it = request.headers().begin();
         it != request.headers().end(); ++it)
    {
        os << "\t" << it->first << ": " << it->second << "\n";
    }

    print_field(os, "Body", request.body());
    return os;
}
