#ifndef CONNECTIONPOOL_HPP
#define CONNECTIONPOOL_HPP

#include "server/Connection.hpp"
#include "server/EventManager.hpp"

class EventManager;

class ConnectionPool
{
  public:
    // construct/destruct
    ConnectionPool(size_t max_connections);
    ~ConnectionPool();

    // api
    EventAction make(const Socket* server_socket, const ServiceConfig& service);
    void remove_idles(EventManager& event_manager);
    void remove(Connection& conn);
    void remove(std::vector<EventAction>& conns_to_remove);
    Connection* contains(const Connection* conn) const;
    Connection* contains(int conn_socket_fd) const;
    bool is_full() const;

  private:
    size_t m_max_conns;
    std::vector<Connection*> m_pool;
};

#endif // CONNECTIONPOOL_HPP
