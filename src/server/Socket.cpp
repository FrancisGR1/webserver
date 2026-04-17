#include <unistd.h>

#include "core/Logger.hpp"
#include "server/Socket.hpp"

Socket::Socket(int fd, const Listener& listener)
    : m_fd(fd)
    , m_listener(listener)
{
    Logger::trace("Socket: construct with fd=%d", m_fd);
}

Socket::~Socket()
{
    Logger::trace("Socket: destruct");
}

int Socket::fd() const
{
    return (m_fd);
}

const Listener& Socket::listener() const
{
    return (m_listener);
}
