#include "Socket.hpp"

Socket::Socket(int fd, const ServiceConfig& service) 
	: fd_(fd)
	, service_(service) {}

Socket::~Socket() {}

int	Socket::getFd() const
{
	return (fd_);
}

const ServiceConfig& Socket::getService() const
{
	return (service_);
}
