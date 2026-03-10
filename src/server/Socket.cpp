#include <unistd.h>

#include "core/Logger.hpp"
#include "Socket.hpp"

Socket::Socket(int fd, const ServiceConfig& service) 
	: fd_(fd)
	, service_(service) 
{
	Logger::trace("Create socket: %d\n", fd_);
}

Socket::~Socket() 
{
	close();
	Logger::trace("Close socket: %d\n", fd_);
}

int	Socket::fd() const
{
	return (fd_);
}

int Socket::close()
{
	if (fd_ < 0)
		return -1;

	return ::close(fd_);
}

const ServiceConfig& Socket::service() const
{
	return (service_);
}
