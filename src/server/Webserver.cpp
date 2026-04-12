#include "Webserver.hpp"
#include "Connection.hpp"
#include "EventManager.hpp"
#include "Socket.hpp"
#include "core/Logger.hpp"
#include "core/utils.hpp"

bool Webserver::is_running = true;

Webserver::Webserver(const Config& config)
    : m_config(config)
    , m_connection_pool(m_events)
{
}

Webserver::~Webserver()
{
    /* fechar sockets do servidor */
    for (std::map<int, Socket*>::iterator it = m_server_sockets.begin(); it != m_server_sockets.end(); it++)
        delete it->second;
}

// if there's an error in setup(), server won't run
void Webserver::setup()
{

    Logger::info("Webserver: setting up sockets");
    for (size_t i = 0; i < m_config.services.size(); i++)
    {
        const ServiceConfig& service = m_config.services[i];
        for (size_t j = 0; j < service.listeners.size(); j++)
        {
            // make server socket
            const Listener& listener = service.listeners[j];
            Socket* socket = make_server_socket(listener, service);

            // store socket
            m_server_sockets.insert(std::pair<int, Socket*>(socket->fd(), socket));
            if (m_events.add(socket->fd(), EPOLLIN) == -1)
            {
                throw std::runtime_error("Failed to add socket to events");
            }

            Logger::info("Webserver: listening on %s:%s", listener.host.c_str(), listener.port.c_str());
        }
    }
}

bool Webserver::isServerSocket(int fd)
{
    return (m_server_sockets.find(fd) != m_server_sockets.end());
}

void Webserver::run()
{
    Logger::info("Webserver: running");

    while (is_running)
    {
        int n_events = m_events.wait();
        if (n_events == -1)
            continue;

        //@TODO colocar try catch dentro do for loop
        for (int i = 0; i < n_events; ++i)
        {
            const epoll_event& event = m_events.get_event(i);
            int event_fd = event.data.fd;

            if (event.events & (EPOLLERR | EPOLLHUP)) // event error @TODO epollhup nem sempre é um erro
            {
                Logger::error("Webserver: Fd %d - something went wrong", event_fd);
                Connection& conn = m_connection_pool.get(event_fd);
                m_connection_pool.remove(conn);
            }
            else if (isServerSocket(event_fd))
            {
                Logger::trace("Webserver: Fd %d is a server socket", event_fd);
                Socket* client_socket = make_client_socket(event_fd);
                m_connection_pool.make(*client_socket);
            }
            else // is an existing connection
            {
                Logger::trace("Webserver: Fd %d is an existing connection", event_fd);
                Connection& conn = m_connection_pool.get(event_fd);
                conn.handle_event(event);
                m_events.apply(conn.give_events(), &conn);
                if (conn.done())
                {
                    m_connection_pool.remove(conn);
                }
            }
        }
    }
}

Socket* Webserver::make_server_socket(const Listener& listener, const ServiceConfig& service)
{
    struct addrinfo hints = {};
    struct addrinfo* result;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(listener.host.c_str(), listener.port.c_str(), &hints, &result) != 0)
    {
        throw std::runtime_error(
            utils::fmt("getaddrinfo() failed for listener %s:%s", listener.host.c_str(), listener.port.c_str()));
    }
    /* criar socket, configurar, bind... */
    int socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    //@TODO substituir por exception de servidor
    //@TODO recuperar antigas mensagens de erros
    if (socket_fd < 0)
    {
        throw std::runtime_error(
            utils::fmt("socket() failed for listener %s:%s", listener.host.c_str(), listener.port.c_str()));
    }

    //@QUESTION opt 1?
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        ::freeaddrinfo(result);
        throw std::runtime_error(
            utils::fmt("setsockopt() failed for listener %s:%s", listener.host.c_str(), listener.port.c_str()));
    }

    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        ::freeaddrinfo(result);
        throw std::runtime_error(
            utils::fmt("fcntl() failed for listener %s:%s", listener.host.c_str(), listener.port.c_str()));
    }

    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) < 0)
    {
        ::freeaddrinfo(result);
        throw std::runtime_error(
            utils::fmt("bind() failed for listener %s:%s", listener.host.c_str(), listener.port.c_str()));
    }
    ::freeaddrinfo(result);

    //@QUESTION porquê 10 aqui?
    if (listen(socket_fd, 10) < 0)
    {
        throw std::runtime_error(
            utils::fmt("listen() failed for listener %s:%s", listener.host.c_str(), listener.port.c_str()));
    }
    return new Socket(socket_fd, service);
}

//@TODO retornar nulo
Socket* Webserver::make_client_socket(int server_socket_fd)
{
    int fd = accept(server_socket_fd, NULL, NULL);
    if (fd < -1)
    {
        Logger::error("Failed to accept client connection!");
    }

    // @QUESTION porquê opt = 0?
    int opt = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < -1)
    {
        Logger::error("setsockopt() for client failed!");
    }

    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -2)
    {
        Logger::error("fcntl() failed to set client socket to non-blocking mode!");
    }

    std::map<int, Socket*>::iterator it = m_server_sockets.find(server_socket_fd);
    if (it == m_server_sockets.end())
        throw std::runtime_error("Server socket not found");

    return new Socket(fd, it->second->service());
}
