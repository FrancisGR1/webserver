#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h> /* sockaddr_in */
#include <poll.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h> /* socket, setsockopt */
#include <unistd.h>

#include "ConnectionPool.hpp"
#include "EventManager.hpp"
#include "Socket.hpp"
#include "config/Config.hpp"

class Connection;

class Webserver
{
  public:
    Webserver(const Config& config);
    ~Webserver();

    void setup();
    void run();

    static bool is_running;

    void startServer();
    int setupSocket();
    bool isServerSocket(int fd);
    void acceptConnection(const int sock);
    void handleConnection(const int sock, epoll_event& event);

    void log(const std::string& message);

  private:
    const Config& m_config;
    std::map<int, Socket*> m_server_sockets;
    EventManager m_events;
    ConnectionPool m_connection_pool;

    // utils
    Socket* make_server_socket(const Listener& listener, const ServiceConfig& service);
    Socket* make_client_socket(int fd);

    // illegal
    Webserver(); // must be initialized with config
};

#endif /* WEBSERVER_HPP */
