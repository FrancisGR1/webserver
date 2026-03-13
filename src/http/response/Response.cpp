#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "core/constants.hpp"
#include "core/utils.hpp"
#include "config/types/Route.hpp"
#include "http/response/Response.hpp"

Response::Response()
	: m_body_fd(-1)
	, m_send_phase(Response::StatusPhase)
	, m_offset(0)
{
	//@QUESTION can we set default headers?
}

Response::Response(StatusCode::Code status)
	: m_status(status)
	, m_status_line(make_status_line())
	, m_body_fd(-1)
	, m_send_phase(Response::StatusPhase)
	, m_offset(0)
{
	//@QUESTION can we set default headers?
}

Response::Response(const Response& other)
	: m_status(other.m_status)
	, m_status_line(other.m_status_line)
	, m_headers_str(other.m_headers_str)
	, m_headers(other.m_headers)
	, m_body_str(other.m_body_str)
	, m_body_fd(other.m_body_fd)
	, m_send_phase(other.m_send_phase)
	, m_offset(other.m_offset) {}

Response& Response::operator=(const Response& other)
{
	if (this != &other)
	{
		m_status = other.m_status;
		m_status_line = other.m_status_line;
		m_headers_str = other.m_headers_str;
		m_headers = other.m_headers;
		m_body_str = other.m_body_str;
		m_body_fd = other.m_body_fd;
		m_send_phase = other.m_send_phase;
		m_offset = other.m_offset;
	}

	return *this;
}

ssize_t Response::send(int fd)
{
	switch (m_send_phase)
	{
		case StatusPhase:
			{
				if (m_status_line.empty())
m_status_line = make_status_line();
				ssize_t sent = ::send(fd, m_status_line.c_str() + m_offset, m_status_line.size() - m_offset, 0);
				if (sent < 0)
					;
				else if (sent + m_offset == m_status_line.size())
				{
					m_send_phase = HeadersPhase;
					m_offset = 0;
					m_headers_str = utils::map_to_str(m_headers);
					m_headers_str += "\n\r";
				}
				else
				{
					m_offset += sent;
				}

				return sent;
			}
		case HeadersPhase:
			{
				ssize_t sent = ::send(fd, m_headers_str.c_str() + m_offset, m_headers_str.size() - m_offset, 0);
				if (sent < 0)
					;
				else if (sent + m_offset == m_headers_str.size())
				{
					m_send_phase = BodyPhase;
					m_offset = 0;
				}
				else
				{
					m_offset += sent;
				}

				return sent;
			}
		case BodyPhase: 
			{
				ssize_t sent = 0;
				if (m_body_fd > -1) // send body from file/pipe
				{
					// m_body_str is used as a leftover for the next call
					if (!m_body_str.empty())
					{
						sent = ::send(fd, m_body_str.c_str(), m_body_str.size(), 0);
						if (sent <= 0)
							return sent; // nothing to send, or error or EAGAIN, which shouldn't happen (@QUESTION: you sure?)

						m_body_str.erase(0, sent);
						if (!m_body_str.empty())
							return sent; // socket is full
					}

					// read and send to socket
					char buffer[constants::read_chunk_size];
					ssize_t read_bytes = ::read(m_body_fd, buffer, constants::read_chunk_size);
					if (read_bytes < 0)
					{
						return -1;
					}
					if (read_bytes == 0)
					{
						m_send_phase = Done;
						return 0;
					}

					sent = ::send(fd, buffer, read_bytes, 0);
					if (sent <= 0)
						return sent;
					if (sent < read_bytes)
					{
						// store leftover for next call
						m_body_str.assign(buffer + sent, read_bytes - sent);
					}

				}
				else if (!m_body_str.empty()) // send body from string
				{
					sent = ::send(fd, m_body_str.c_str() + m_offset, m_body_str.size() - m_offset, 0);
					if (sent < 0)
						return sent;

					m_offset += sent;
					if (m_offset == m_body_str.size())
						m_send_phase = Done;
				}
				else // no body
				{
					m_send_phase = Done;
				}

				return sent;
			}
		default:
			{
				return 0;
			}
	}
}

bool Response::done() const
{
	return m_send_phase == Done;
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
	m_status = status;
}

void Response::set_header(const std::string& key, const std::string& value)
{
	m_headers[key] = value;
}

void Response::set_body_as_str(const std::string& str)
{
	m_body_str = str;
}

// prefix is something you might want to send after the headers and before reading the file, for example a read() call that processed the headers and a bit of the body
void Response::set_body_as_fd(int fd, const std::string& prefix)
{
	m_body_fd = fd;
	m_body_str = prefix;
}

void Response::set_body_as_path(const Path& path)
{
	if (path.exists)
	{
		//@QUESTION @TODO add to the event loop?
		m_body_fd = open(path.raw.c_str(), O_RDONLY);
	}
}

StatusCode::Code Response::status() const { return m_status; }

Response::~Response()
{
	if (m_body_fd >= 0) 
		close (m_body_fd);
}

std::string Response::make_status_line()
{
	std::string status_line;

	// version
	status_line = constants::server_http_version;
	status_line += " ";

	// code
	std::stringstream ss;
	ss << m_status;
	status_line += ss.str() + " ";

	// reason
	status_line += StatusCode::to_reason(m_status) + "\r\n";

	return status_line;
}

std::ostream& operator<<(std::ostream& os, const Response& response)
{
	os << "Response:\n";
	os << "Is done: " << std::boolalpha << response.done() << std::endl
		<< response.m_status_line 
		<< response.m_headers_str;
	if (response.m_body_fd >= 0)
	{
		os << "Body is fd: " << response.m_body_fd << "\n";
	}
	else
	{
		os << response.m_body_str;
	}
	return os;
}
