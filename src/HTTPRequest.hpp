#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <string>
#include <ostream>
#include <map>

class HTTPRequest
{
	public:
		HTTPRequest();
		HTTPRequest(const std::string& method,
				const std::string& target,
				const std::string& protocol_version,
				const std::map<std::string, std::string>& headers,
				const std::string& body);

		const std::string& method() const;
		const std::string& target() const;
		const std::string& protocol_version() const;
		const std::map<std::string, std::string>& headers() const;
		const std::string& body() const;

		const std::string& contains_header(std::string header) const;
	
	private:
		std::string m_method;
		std::string m_target;
		std::string m_protocol_version;
		std::map<std::string, std::string> m_headers;
		std::string m_body;

		void normalize_header(std::string& str);
};

std::ostream& operator<<(std::ostream& os, const HTTPRequest& request);

#endif // HTTPREQUEST_HPP
