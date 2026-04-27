#include <fcntl.h>
#include <sys/socket.h>

#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "core/contracts.hpp"
#include "core/utils.hpp"
#include "server/Connection.hpp"
#include "server/ConnectionPool.hpp"
#include "server/EventAction.hpp"
#include "server/EventManager.hpp"

// constructor
ConnectionPool::ConnectionPool(size_t max_connections)
    : m_pool(max_connections, NULL)
{
    Logger::verbose("ConnectionPool: constructor");
}

// destructor
ConnectionPool::~ConnectionPool()
{
    Logger::trace("ConnectionPool: destructor");
}

// api
// make connection, store it, return an event
EventAction ConnectionPool::make(const Socket* server_socket, const ServiceConfig& service)
{
    REQUIRE(server_socket != NULL, "Socket is Null!");

    Logger::trace("ConnectionPool: make client socket from: fd='%d'", server_socket->fd());

    if (is_full())
        throw std::runtime_error(utils::fmt("Connection pool limit reached: %zu", m_pool.size()));
    // create client socket
    int client_fd = ::accept(server_socket->fd(), NULL, NULL);
    if (client_fd == -1)
    {
        throw std::runtime_error("Failed to accept client connection!");
    }

    // set socket options
    int opt = 0;
    if (setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        throw std::runtime_error("setsockopt() for client failed!");
    }
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        throw std::runtime_error("fcntl() failed to set client socket to non-blocking mode!");
    }

    // create connection
    for (size_t i = 0; i < m_pool.size(); ++i)
    {
        if (m_pool[i] == NULL)
        {
            //@NOTE server_socket listener goes into client socket listener to preserve the info for the cgi later
            m_pool[i] = new Connection(client_fd, server_socket->listener(), service);
            return EventAction(EventAction::WantRead, EventAction::ClientSocket, m_pool[i]->socket().fd(), m_pool[i]);
        }
    }

    INVARIANT(false, "No slot available found for new connection!");
    return EventAction(EventAction::WantClose, EventAction::ServerSocket, -1, NULL); // unreachable
}

void ConnectionPool::remove_idles(EventManager& event_manager)
{
    std::time_t now = Timer::now();

    for (size_t i = 0; i < m_pool.size(); ++i)
    {
        if (!m_pool[i])
            continue;
        if ((now - m_pool[i]->last_activity()) > constants::idle_connection_timeout)
        {
            Logger::debug("ConnectionPool: removing idle connection: '%p'", (void*)m_pool[i]);

            if (m_pool[i]->state() == ConnectionState::Reading)
                m_pool[i]->send_error(StatusCode::RequestTimeout);
            else if (m_pool[i]->state() == ConnectionState::ProcessingRequest)
                m_pool[i]->send_error(StatusCode::ServiceUnavailable);
            else
            {
                // send nothing
            };

            EventAction ea(EA::WantClose, EA::ClientSocket, m_pool[i]->fd(), m_pool[i]);
            event_manager.apply(ea);
            delete m_pool[i];
            m_pool[i] = NULL;
        }
    }
}

void ConnectionPool::remove(Connection& conn)
{
    for (size_t i = 0; i < m_pool.size(); ++i)
    {
        if (m_pool[i] == &conn)
        {
            delete m_pool[i];
            m_pool[i] = NULL; // mark free slot
            Logger::debug("ConnectionPool: Connection[%zu] removed from pool slot", 0);
            return;
        }
    }
    Logger::warn("ConnectionPool: Attempted to remove connection not in pool");
}

void ConnectionPool::remove(std::vector<EventAction>& conns_to_remove)
{
    for (size_t i = 0; i < conns_to_remove.size(); ++i)
    {
        EventAction& event = conns_to_remove[i];
        if (event.action == EventAction::WantClose)
        {
            INVARIANT(event.conn != NULL, "Connection must always be non null");
            remove(*event.conn);
        }
    }
}

Connection* ConnectionPool::contains(const Connection* conn) const
{
    for (size_t i = 0; i < m_pool.size(); ++i)
    {
        if (m_pool[i] != NULL && conn == m_pool[i])
            return m_pool[i];
    }

    return NULL;
}

Connection* ConnectionPool::contains(int conn_socket_fd) const
{
    for (size_t i = 0; i < m_pool.size(); ++i)
    {
        if (m_pool[i] != NULL && conn_socket_fd == m_pool[i]->socket().fd())
        {
            Logger::trace("ConnectionPool: contains fd %d", conn_socket_fd);
            return m_pool[i];
        }
    }

    return NULL;
}

bool ConnectionPool::is_full() const
{
    for (size_t i = 0; i < m_pool.size(); ++i)
    {
        if (m_pool[i] == NULL)
            return false;
    }
    Logger::info("ConnectionPool: is full!");
    return true;
}
