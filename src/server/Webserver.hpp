#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "config/Config.hpp"
#include "server/ConnectionPool.hpp"
#include "server/EventManager.hpp"
#include "server/Socket.hpp"

class Connection;

class Webserver
{
  public:
    // constructor/destructor
    Webserver(const Config& config);
    ~Webserver();

    // api
    void setup();
    void run();

    // for CTRL+C
    static volatile bool is_running;

  private:
    const Config& m_config;
    std::map<int, Socket*> m_server_sockets;
    ConnectionPool m_connection_pool;
    EventManager m_events;

    // utils
    Socket* make_server_socket(const Listener& listener);
    Socket* get_server_socket(const EventAction& ea);
    const ServiceConfig& get_service(const Socket* server_socket);
    static void handle_sigint(int sig);

    // illegal
    Webserver(); // must be initialized with config
};

#endif /* WEBSERVER_HPP */
