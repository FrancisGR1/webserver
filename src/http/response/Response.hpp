#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <map>
#include <string>
#include <sys/types.h>

#include "config/types/Route.hpp"
#include "core/Path.hpp"
#include "http/StatusCode.hpp"

class Response
{
  public:
    // construct/copy/destruct
    Response();
    explicit Response(StatusCode::Code status);
    Response(StatusCode::Code status, std::map<std::string, std::string> m_headers, std::string body);
    Response(const Response& other);
    Response& operator=(const Response& other);
    ~Response();

    // api
    ssize_t send(int fd);
    bool done() const;

    // makers
    void make_redirection_response(StatusCode::Code status, const Route& redirection);

    // setters
    void set_status(StatusCode::Code status);
    void set_header(const std::string& key, const std::string& value);
    void set_body_as_str(const std::string& str);
    void set_body_as_fd(int fd);
    int set_body_as_path(const Path& path);

    // getters
    StatusCode::Code status_code() const;
    const std::string& body() const;
    int body_fd() const;
    const std::map<std::string, std::string>& headers() const;

    // overloads
    bool operator==(const Response& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Response& response);

  private:
    // status, headers, body
    StatusCode::Code m_status;
    std::string m_status_line;
    std::string m_headers_str;
    std::map<std::string, std::string> m_headers;
    // body is either one or both of the following
    // if it's both, m_body_str is treated as a prefix
    std::string m_body_str;
    int m_body_fd; // borrows

    // transmission state
    enum SendPhase
    {
        StatusPhase = 0,
        HeadersPhase,
        BodyPhase,
        Done
    };
    SendPhase m_send_phase;
    size_t m_offset;
    size_t m_total_sent;

    // utils
    std::string make_status_line();
};

std::ostream& operator<<(std::ostream& os, const Response& response);

#endif // RESPONSE_HPP
