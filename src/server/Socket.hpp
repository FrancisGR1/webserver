#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "config/types/Listener.hpp"

class Socket
{
  public:
    // construct/destruct
    Socket(int fd, const Listener& listener);
    ~Socket();

    // getters
    int fd() const;
    const Listener& listener() const;

  private:
    int m_fd;
    Listener m_listener;

    // illegal
    Socket(const Socket&);
    Socket& operator=(const Socket&);
};

#endif /* SOCKET_HPP */
