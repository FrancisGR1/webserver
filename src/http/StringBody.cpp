#include <sys/socket.h>

#include "StringBody.hpp"

StringBody::StringBody(const std::string&  str)
	: m_body(str)
	, m_bytes_sent(0) {}

StringBody::StringBody(const char* str)
	: m_body(str)
	, m_bytes_sent(0) {}

ssize_t StringBody::send(int fd)
{
	if (done())
		return 0;
	
	const char* data = m_body.c_str() + m_bytes_sent;
	int remaining = m_body.size() - m_bytes_sent;
	if (remaining <= 0)
		return 0;

	ssize_t sent = ::send(fd, data, remaining, 0);
	if (sent > 0)
		m_bytes_sent += sent;

	return sent;
}

size_t StringBody::size() const { return m_body.size(); }
bool StringBody::done() const { return m_bytes_sent == m_body.size(); }

StringBody::~StringBody() {};
