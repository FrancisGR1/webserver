#include <unistd.h>

#include "Socket.hpp"
#include "core/Logger.hpp"

Socket::Socket(int fd, const Listener& listener)
    : m_fd(fd)
    , m_listener(listener)
{
    Logger::trace("Socket: construct with fd=%d", m_fd);
}

Socket::~Socket()
{
    close();
    Logger::trace("Socket: destruct with fd=%d", m_fd);
}

int Socket::fd() const
{
    return (m_fd);
}

int Socket::close()
{
    if (m_fd < 0)
        return -1;

    return ::close(m_fd);
}

const Listener& Socket::listener() const
{
    return (m_listener);
}
