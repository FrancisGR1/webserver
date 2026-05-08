#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <map>
#include <string>
#include <sys/types.h>

#include "core/macros.hpp"
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
    ssize_t send(int socket_fd);
    bool done() const;

    // makers
    void make_redirection_response(StatusCode::Code status, const Route& redirection);

    // setters
    void set_status(StatusCode::Code status);
    void set_header(const std::string& key, const std::string& value);
    void set_body_as_str(const std::string& str);
    void set_body_as_fd(int fd);
    int set_body_as_path(Path& path);

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
    Path m_body_path;

    // transmission state
    enum State
    {
        Status = 0,
        Headers,
        Body,
        Done
    };
    State m_state;
    size_t m_offset;
    size_t m_total_sent;

    // utils
    //-send
    //--status
    ssize_t send_status_line(int socket_fd);
    std::string make_status_line();
    //--headers
    ssize_t send_headers(int socket_fd);
    //--body
    ssize_t send_body(int socket_fd);
    ssize_t send_body_from_fd(int socket_fd);
    ssize_t send_body_from_str(int socket_fd);
    //-state
    void next_state(State state);
};

std::ostream& operator<<(std::ostream& os, const Response& response);

#endif // RESPONSE_HPP
