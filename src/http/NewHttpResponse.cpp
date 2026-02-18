#include <sys/socket.h>

#include "core/constants.hpp"
#include "NewHttpResponse.hpp"

NewHttpResponse::NewHttpResponse(StatusCode::Code status)
	: m_status(status)
	, m_body(NULL) 
	, m_send_phase(NewHttpResponse::StatusPhase)
	, m_offset(0)
{
	//@TODO default headers
}

void NewHttpResponse::set_header(const std::string& key, const std::string& value)
{
	m_headers[key] = value;
}

void NewHttpResponse::set_body(IBody* body)
{
	delete m_body;
	m_body = body;
}

StatusCode::Code NewHttpResponse::status() const { return m_status; }

ssize_t NewHttpResponse::send(int fd) const
{
	switch (m_send_phase)
	{
		case StatusPhase:
			std::string status_line =  constants::server_http_version + " " +
						   m_status + " " +
						   StatusCode::to_string(m_status) + "\r\n";
			return ::send(fd, status_line.c_str(), status_line.size(), 0);

		case HeadersPhase:
			std::string headers = utils::map_to_str(m_headers);
			return ::send(fd, headers.c_str(), headers.size(), 0);

		case BodyPhase: 
			return m_body->send(fd);
	}

	return 0;
}

bool NewHttpResponse::done() const
{
	return m_send_phase == SendPhase::Done;
}

NewHttpResponse::~NewHttpResponse()
{
	delete m_body;
}
