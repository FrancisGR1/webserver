#include <csignal>

#include "core/Logger.hpp"
#include "core/contracts.hpp"
#include "server/Connection.hpp"
#include "server/EventAction.hpp"
#include "server/EventManager.hpp"
#include "server/Socket.hpp"
#include "server/Webserver.hpp"

volatile bool Webserver::is_running = true;

Webserver::Webserver(const Config& config)
    : m_config(config)
    , m_server_sockets()
    , m_connection_pool(constants::max_connections)
    , m_events(constants::max_events, m_connection_pool)
{
    Logger::verbose("Webserver: constructor");
}

Webserver::~Webserver()
{
    Logger::trace("Webserver: destructor");

    // close sockets
    for (std::map<int, Socket*>::iterator it = m_server_sockets.begin(); it != m_server_sockets.end(); it++)
        delete it->second;
}

// if there's an error in setup(), server won't run
void Webserver::setup()
{
    // signals
    ::signal(SIGINT, Webserver::handle_sigint);
    ::signal(SIGPIPE, SIG_IGN);

    Logger::info("Webserver: setting up sockets");
    for (size_t i = 0; i < m_config.services.size(); i++)
    {
        const ServiceConfig& service = m_config.services[i];
        for (size_t j = 0; j < service.listeners.size(); j++)
        {
            // make server socket
            const Listener& listener = service.listeners[j];
            Socket* socket = make_server_socket(listener);
            if (socket == NULL) // skip
                continue;

            // store socket
            m_server_sockets.insert(std::pair<int, Socket*>(socket->fd(), socket));
            EventAction ea(EventAction::WantRead, EventAction::ServerSocket, socket->fd(), NULL);
            if (m_events.apply(ea) == -1)
            {
                Logger::warn("Webserver: failed to add socket to events, skipping");
                delete socket;
                continue;
            }

            Logger::info(
                "Webserver: listening on %s:%s", socket->listener().host.c_str(), socket->listener().port.c_str());
        }
    }

    if (m_server_sockets.empty())
        throw std::runtime_error("Webserver: no listeners could be bound, aborting");
}

void Webserver::run()
{
    Logger::info("Webserver: running");

    while (is_running)
    {
        int n_events = m_events.wait();
        if (n_events == -1)
            continue;

        EventAction* event = NULL;

        while (m_events.next(event))
        {
            Connection* conn = event->conn;
            if (conn)
                conn->set(*event);
            try
            {
                switch (event->type)
                {
                    case EventAction::ServerSocket:
                    {
                        Logger::trace("Webserver: Fd %d is a server socket", event->fd);
                        const Socket* ss = get_server_socket(*event);
                        const ServiceConfig& service = get_service(ss);
                        const EventAction& ec = m_connection_pool.make(ss, service);
                        m_events.apply(ec);
                        continue;
                    }
                    case EventAction::Pipe:
                    {
                        INVARIANT(conn != NULL, "Connection should never be null if it's not a server socket!");
                        Logger::trace(
                            "Webserver: Connection wants to '%s': fd='%d'",
                            event->action == EventAction::WantRead ? "read from" : "write to",
                            event->fd);

                        conn->process_request();

                        break;
                    }
                    case EventAction::ClientSocket:
                    {
                        INVARIANT(conn != NULL, "Connection should never be null if it's not a server socket!");

                        switch (event->action)
                        {
                            case EventAction::WantRead:
                            {
                                Logger::trace(
                                    "Webserver: Connection wants to read to: '%s'",
                                    event->type == EventAction::Pipe ? "pipe" : "socket");

                                conn->read();

                                break;
                            }
                            case EventAction::WantProcessRequest:
                            {
                                Logger::trace("Webserver: Connection wants to process request");

                                conn->process_request();

                                break;
                            }
                            case EventAction::WantWrite:
                            {
                                Logger::trace(
                                    "Webserver: Connection wants to write to: '%s'",
                                    event->type == EventAction::Pipe ? "pipe" : "socket");

                                conn->write();

                                break;
                            }
                            case EventAction::WantClose: //@NOTE rare path, only happens if something went wrong with
                                                         // the
                                                         // fds
                            {
                                Logger::trace("Webserver: Connection wants to be closed");

                                if (event->type == EventAction::ClientSocket)
                                    m_connection_pool.remove(*conn);

                                break;
                            }
                        }
                    }
                }

                m_events.apply(conn->give_events());
            }
            catch (const std::exception& error)
            {
                Logger::error("Webserver: '%s'", error.what());
                if (conn)
                {
                    conn->send_error_and_close(StatusCode::InternalServerError);
                }
            }
        }
    }

    Logger::info("Webserver: stop running");
}

Socket* Webserver::make_server_socket(const Listener& listener)
{
    struct addrinfo hints = {};
    struct addrinfo* result;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(listener.host.c_str(), listener.port.c_str(), &hints, &result) != 0)
    {
        Logger::warn(
            "Webserver: getaddrinfo() failed for listener %s:%s, skipping",
            listener.host.c_str(),
            listener.port.c_str());
        return NULL;
    }

    int socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd < 0)
    {
        ::freeaddrinfo(result);
        Logger::warn(
            "Webserver: socket() failed for listener %s:%s, skipping", listener.host.c_str(), listener.port.c_str());
        return NULL;
    }

    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ||
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        ::close(socket_fd);
        ::freeaddrinfo(result);
        Logger::warn(
            "Webserver: setsockopt() failed for listener %s:%s, skipping",
            listener.host.c_str(),
            listener.port.c_str());
        return NULL;
    }

    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        ::close(socket_fd);
        ::freeaddrinfo(result);
        Logger::warn(
            "Webserver: fcntl() failed for listener %s:%s, skipping", listener.host.c_str(), listener.port.c_str());
        return NULL;
    }

    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) < 0)
    {
        ::close(socket_fd);
        ::freeaddrinfo(result);
        Logger::warn(
            "Webserver: bind() failed for listener %s:%s, skipping", listener.host.c_str(), listener.port.c_str());
        return NULL;
    }
    ::freeaddrinfo(result);

    if (listen(socket_fd, 10) < 0)
    {
        ::close(socket_fd);
        Logger::warn(
            "Webserver: listen() failed for listener %s:%s, skipping", listener.host.c_str(), listener.port.c_str());
        return NULL;
    }

    return new Socket(socket_fd, listener);
}

Socket* Webserver::get_server_socket(const EventAction& ea)
{
    REQUIRE(ea.type == EventAction::ServerSocket, "EventAction must be from server socket");

    return m_server_sockets.find(ea.fd)->second;
}

const ServiceConfig& Webserver::get_service(const Socket* server_socket)
{
    REQUIRE(server_socket != NULL, "Socket is Null!");

    for (size_t i = 0; i < m_config.services.size(); ++i)
    {
        const ServiceConfig& service = m_config.services[i];
        for (size_t j = 0; j < service.listeners.size(); ++j)
        {
            const Listener& config_listener = service.listeners[j];
            if (server_socket->listener() == config_listener)
                return service;
        }
    }

    INVARIANT(false, "No matching service found for socket");
    return m_config.services[0]; // unreachable
}

void Webserver::handle_sigint(int sig)
{
    (void)sig;
    Webserver::is_running = false;
    ::write(STDOUT_FILENO, "\n", 1);
}
