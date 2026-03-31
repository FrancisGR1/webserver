#include <unistd.h>

#include "Socket.hpp"
#include "core/Logger.hpp"

Socket::Socket(int fd, const ServiceConfig& service)
    : fd_(fd)
    , service_(service)
{
    Logger::trace("Socket: construct with fd=%d", fd_);
}

Socket::~Socket()
{
    close();
    Logger::trace("Socket: destruct with fd=%d", fd_);
}

int Socket::fd() const
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
