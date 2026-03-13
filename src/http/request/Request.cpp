#include <string>
#include <ostream>
#include <map>
#include <iomanip>

#include "http/StatusCode.hpp"
#include "http/request/Request.hpp"

//@TODO: impl
Request::Request() {}

Request::Request(const std::string& method,
			const std::string& target_path,
			const std::string& target_query,
			const std::string& protocol_version,
			const std::map<std::string, std::string>& headers,
			const std::string& body, 
			StatusCode::Code c)
	: m_method(method)
	, m_target_path(target_path)
	, m_target_query(target_query)
	, m_protocol_version(protocol_version)
	, m_headers(headers)
	, m_body(body)
	, m_status_code(c)
{}

const std::string& Request::method() const { return m_method; }
const std::string& Request::target_path() const { return m_target_path; }
const std::string& Request::target_query() const { return m_target_query; }
const std::string& Request::protocol_version() const { return m_protocol_version; }
const std::map<std::string, std::string>& Request::headers() const { return m_headers; }
const std::string& Request::body() const { return m_body; }
bool Request::bad_request() const { return m_status_code != StatusCode::Ok; }
StatusCode::Code Request::status_code() const { return m_status_code; }

static void print_field(std::ostream& os, const std::string& title, const std::string& value, int width = 14)
{
    os << std::left << std::setw(width) << (title + ": ") << value << "\n";
}

std::ostream& operator<<(std::ostream& os, const Request& request)
{
    os << "Request:\n";
    print_field(os, "Method",  request.method(), 0);
    print_field(os, "Target Path",  request.target_path(), 0);
    print_field(os, "Target Query",  request.target_query(), 0);
    print_field(os, "Version", request.protocol_version(), 0);

    os << std::left << std::setw(10) << "Headers:" << "\n";
    for (std::map<std::string,std::string>::const_iterator it = request.headers().begin();
         it != request.headers().end(); ++it)
    {
        os << "\t" << it->first << ": " << it->second << "\n";
    }

    print_field(os, "Body", request.body());
    return os;
}
