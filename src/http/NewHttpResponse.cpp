#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>

#include "core/constants.hpp"
#include "core/utils.hpp"
#include "NewHttpResponse.hpp"

NewHttpResponse::NewHttpResponse()
	: m_body_fd(-1)
	, m_send_phase(NewHttpResponse::StatusPhase)
	, m_offset(0)
{
	//@QUESTION can we set default headers?
}

NewHttpResponse::NewHttpResponse(StatusCode::Code status)
	: m_status(status)
	, m_status_line(make_status_line())
	, m_body_fd(-1)
	, m_send_phase(NewHttpResponse::StatusPhase)
	, m_offset(0)
{
	//@QUESTION can we set default headers?
}

void NewHttpResponse::set_status(StatusCode::Code status)
{
	m_status = status;
}

void NewHttpResponse::set_header(const std::string& key, const std::string& value)
{
	m_headers[key] = value;
}

void NewHttpResponse::set_body(const std::string& str)
{
	m_body_str = str;
}

void NewHttpResponse::set_body(int fd, const std::string prefix = "")
{
	m_body_fd = fd;
	m_body_str = prefix;
}

StatusCode::Code NewHttpResponse::status() const { return m_status; }

ssize_t NewHttpResponse::send(int fd)
{
	switch (m_send_phase)
	{
		case StatusPhase:
			{
				ssize_t sent = ::send(fd, m_status_line.c_str() + m_offset, m_status_line.size() - m_offset, 0);
				if (sent + m_offset == m_status_line.size())
				{
					m_send_phase = HeadersPhase;
					m_offset = 0;
					m_headers_str = utils::map_to_str(m_headers);
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
				if (sent + m_offset == m_headers_str.size())
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
				return (m_body ? m_body->send(fd) : 0);
			}
		default:
			{
				return 0;
			}
	}
}

bool NewHttpResponse::done() const
{
	return m_send_phase == Done;
}

NewHttpResponse::~NewHttpResponse()
{
	delete m_body;
}

std::string NewHttpResponse::make_status_line()
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
	status_line += StatusCode::to_string(m_status) + "\r\n";

	return status_line;
}
