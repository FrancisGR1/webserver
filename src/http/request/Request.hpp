#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <ostream>
#include <string>

#include "http/StatusCode.hpp"

class Request
{
  public:
    //@TODO copy assignment
    Request();
    Request(
        const std::string& method,
        const std::string& target_path,
        const std::string& target_query,
        const std::string& protocol_version,
        const std::map<std::string, std::string>& headers,
        const std::string& body,
        StatusCode::Code c);

    // getters
    const std::string& method() const;
    const std::string& target_path() const;
    const std::string& target_query() const;
    const std::string& protocol_version() const;
    const std::map<std::string, std::string>& headers() const;
    const std::string& body() const;
    bool bad_request() const;
    StatusCode::Code status_code() const;

    bool operator==(const Request& other);

  private:
    std::string m_method;
    std::string m_target_path;
    std::string m_target_query;
    std::string m_protocol_version;
    std::map<std::string, std::string> m_headers;
    std::string m_body;
    StatusCode::Code m_status_code;
};

std::ostream& operator<<(std::ostream& os, const Request& request);

#endif // REQUEST_HPP
