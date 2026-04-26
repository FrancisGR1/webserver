#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sstream>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "config/types/Route.hpp"
#include "core/ResourceLocker.hpp"
#include "core/constants.hpp"
#include "core/contracts.hpp"
#include "core/utils.hpp"
#include "http/response/Response.hpp"

Response::Response()
    : m_status(StatusCode::None)
    , m_body_fd(-1)
    , m_state(Response::Status)
    , m_offset(0)
    , m_total_sent(0)
{
    Logger::trace("Response: constructor");
}

Response::Response(StatusCode::Code status)
    : m_status(status)
    , m_body_fd(-1)
    , m_state(Response::Status)
    , m_offset(0)
    , m_total_sent(0)
{
    Logger::trace("Response: status constructor");
}

Response::Response(StatusCode::Code status, std::map<std::string, std::string> headers, std::string body)
    : m_status(status)
    , m_headers(headers)
    , m_body_str(body)
    , m_body_fd(-1)
    , m_state(Response::Status)
    , m_offset(0)
    , m_total_sent(0)
{
    Logger::trace("Response: status/headers/body constructor");
}

Response::Response(const Response& other)
    : m_status(other.m_status)
    , m_status_line(other.m_status_line)
    , m_headers_str(other.m_headers_str)
    , m_headers(other.m_headers)
    , m_body_str(other.m_body_str)
    , m_body_fd(other.m_body_fd)
    , m_state(other.m_state)
    , m_offset(other.m_offset)
    , m_total_sent(other.m_total_sent)
{
    Logger::trace("Response: copy constructor");
}

Response& Response::operator=(const Response& other)
{
    Logger::trace("Response: assign operator");

    if (this != &other)
    {
        m_status = other.m_status;
        m_status_line = other.m_status_line;
        m_headers_str = other.m_headers_str;
        m_headers = other.m_headers;
        m_body_str = other.m_body_str;
        m_body_fd = other.m_body_fd;
        m_state = other.m_state;
        m_offset = other.m_offset;
        m_total_sent = other.m_total_sent;
    }

    return *this;
}

Response::~Response()
{
    Logger::trace("Response: destructor");

    if (m_body_path.exists)
        ResourceLocker::unlock(m_body_path);
    if (m_body_fd != -1)
        ::close(m_body_fd);
}

ssize_t Response::send(int socket_fd)
{
    switch (m_state)
    {
        case Status: return send_status_line(socket_fd);
        case Headers: return send_headers(socket_fd);
        case Body: return send_body(socket_fd);

        default:
        {
            INVARIANT(false, "Should never reach here. Weird");
            return 0;
        }
    }
}

bool Response::done() const
{
    return m_state == Done;
}

void Response::make_redirection_response(StatusCode::Code status, const Route& redirection)
{
    set_status(status);
    set_header("Location", redirection.raw_path);
    set_header("Connection", "close");
    set_header("Date", utils::http_date());
}

void Response::set_status(StatusCode::Code status)
{
    Logger::trace("Response: set status to: '%d'", status);
    m_status = status;
}

void Response::set_header(const std::string& key, const std::string& value)
{
    Logger::trace("Response: set header: ['%s']='%s'", key.c_str(), value.c_str());
    m_headers[key] = value;
}

void Response::set_body_as_str(const std::string& str)
{
    Logger::trace("Response: set body str: '%s'", str.c_str());
    m_body_str = str;
}

void Response::set_body_as_fd(int fd)
{
    Logger::trace("Response: set body fd: '%d'", fd);
    m_body_fd = fd;
}

int Response::set_body_as_path(Path& path)
{
    REQUIRE(path.exists == true);

    Logger::trace("Response: set body path: '%s'", path.raw.c_str());
    m_body_fd = path.open(O_RDONLY);
    fcntl(m_body_fd, F_SETFL, O_NONBLOCK);
    m_body_path = path;

    return m_body_fd;
}

StatusCode::Code Response::status_code() const
{
    return m_status;
}

const std::string& Response::body() const
{
    return m_body_str;
}

int Response::body_fd() const
{
    return m_body_fd;
}

const std::map<std::string, std::string>& Response::headers() const
{
    return m_headers;
}

//@REFACTOR enviar status_line + headers
ssize_t Response::send_status_line(int socket_fd)
{
    ENSURE(m_state == Response::Status);

    Logger::trace("Response: send status");

    m_status_line = make_status_line();

    ssize_t sent = ::send(socket_fd, m_status_line.c_str() + m_offset, m_status_line.size() - m_offset, 0);
    if (sent == -1)
    {
        Logger::warn("Response: sent %ld bytes: errno says: '%s'", sent, ::strerror(errno));
        next_state(Done);
    }
    else if (sent + m_offset == m_status_line.size())
    {
        next_state(Headers);
        m_offset = 0;
        m_headers_str = utils::map_to_str(m_headers);
        m_headers_str += constants::crlf;
    }
    else
    {
        m_offset += sent;
    }

    m_total_sent += sent;

    return sent;
}

std::string Response::make_status_line()
{
    std::string status_line;

    // version
    status_line = constants::server_http_version;
    status_line += " ";

    // status code
    std::stringstream ss;
    ss << m_status;
    status_line += ss.str() + " ";

    // reason
    status_line += StatusCode::to_reason(m_status) + constants::crlf;

    return status_line;
}

ssize_t Response::send_headers(int socket_fd)
{
    ENSURE(m_state == Response::Headers);
    Logger::trace("Response: send headers");

    ssize_t sent = ::send(socket_fd, m_headers_str.c_str() + m_offset, m_headers_str.size() - m_offset, 0);
    if (sent == -1)
    {
        Logger::warn("Response: sent %ld bytes: errno says: '%s'", sent, ::strerror(errno));
        next_state(Done);
    }
    else if (sent + m_offset == m_headers_str.size())
    {
        next_state(Body);
        m_offset = 0;
    }
    else
    {
        m_offset += sent;
    }

    m_total_sent += sent;

    return sent;
}

ssize_t Response::send_body(int socket_fd)
{
    ENSURE(m_state == Response::Body);
    Logger::trace("Response: send body");

    if (m_body_fd > -1)
    {
        return send_body_from_fd(socket_fd);
    }
    else if (!m_body_str.empty())
    {
        return send_body_from_str(socket_fd);
    }
    else // no body
    {
        Logger::trace("Response: done!");
        next_state(Done);
        return 0;
    }
}

ssize_t Response::send_body_from_fd(int socket_fd)
{
    Logger::debug("Response: send body from fd=%d", m_body_fd);

    ssize_t sent_bytes = 0;
    // m_body_str is used as a leftover for the next call
    if (!m_body_str.empty())
    {
        return send_body_from_str(socket_fd);
    }

    // @TODO: substitute read() -> send() with sendfile()
    // read
    char buffer[constants::read_chunk_size + 1] = {};
    ssize_t read_bytes = ::read(m_body_fd, buffer, constants::read_chunk_size);
    Logger::trace("Response: read %ld bytes: '%s'", read_bytes, buffer);
    if (read_bytes < 0)
    {
        Logger::warn("Response: errno: '%s'", ::strerror(errno));
        return -1;
    }
    if (read_bytes == 0)
    {
        next_state(Done);
        return 0;
    }
    buffer[read_bytes] = '\0';

    // send
    sent_bytes = ::send(socket_fd, buffer, read_bytes, 0);
    Logger::trace("Response: sent %ld(%ld) bytes", sent_bytes, m_total_sent);
    if (sent_bytes < read_bytes)
    {
        // store leftover for next call
        if (sent_bytes > 0)
            m_body_str.assign(buffer + sent_bytes, read_bytes - sent_bytes);
        else // buffer is full, save everything
            m_body_str.assign(buffer, read_bytes);
    }
    if (sent_bytes == -1)
    {
        Logger::warn("Response: send() error: errno says: '%s'", strerror(errno));
        next_state(Done);
        return -1;
    }
    if (sent_bytes == 0)
    {
        Logger::warn("Response: sent 0 bytes");
        return 0;
    }

    // add to total
    m_total_sent += sent_bytes;

    return sent_bytes;
}

ssize_t Response::send_body_from_str(int socket_fd)
{
    Logger::debug(
        "Response: send body (string[%zu]): %s", m_body_str.size() - m_offset, (m_body_str.c_str() + m_offset));

    ssize_t sent_bytes = ::send(socket_fd, m_body_str.c_str() + m_offset, m_body_str.size() - m_offset, 0);
    if (sent_bytes < 0)
        return sent_bytes;

    m_total_sent += sent_bytes;

    m_offset += sent_bytes;
    if (m_offset == m_body_str.size())
        next_state(Done);

    return sent_bytes;
}

void Response::next_state(Response::State state)
{
    m_state = state;
}

bool Response::operator==(const Response& other) const
{
    // clang-format off
	return ( 
		m_status   == other.m_status &&
		m_headers  == other.m_headers &&
		(m_body_str == other.m_body_str || 
		(m_body_str == other.m_body_str.substr(0, m_body_str.size())  && m_body_fd != -1)) // half string half file bodies
	       );
    // clang-format on
}

std::ostream& operator<<(std::ostream& os, const Response& response)
{
    os << "Is done: " << std::boolalpha << response.done() << std::endl;
    os << "Status:  '" << response.m_status_line << "'\n";
    os << "Headers str:  '" << response.m_headers_str << "'\n";
    os << "Headers map:\n";
    for (std::map<std::string, std::string>::const_iterator it = response.m_headers.begin();
         it != response.m_headers.end();
         ++it)
    {
        os << "\t'" << it->first << "': '" << it->second << "'\n";
    }
    os << "Body fd:  " << response.m_body_fd << "\n";
    os << "Body str: '" << response.m_body_str << "'\n";
    return os;
}
