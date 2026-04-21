#ifndef CONNECTIONPOOL_HPP
#define CONNECTIONPOOL_HPP

#include "server/Connection.hpp"

class ConnectionPool
{
  public:
    // construct/destruct
    ConnectionPool(size_t max_connections);
    ~ConnectionPool();

    // api
    EventAction make(const Socket* server_socket, const ServiceConfig& service);
    void remove_idles(void);
    void remove(Connection& conn);
    Connection* contains(const Connection* conn) const;
    Connection* contains(int conn_socket_fd) const;
    bool is_full() const;

  private:
    size_t m_max_conns;
    //@TODO substituir por std::priority_queue e organizar por "last activity"
    std::vector<Connection*> m_pool;
};

#endif // CONNECTIONPOOL_HPP
