#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "http/processor/RequestProcessor.hpp"
#include "http/request/RequestParser.hpp"
#include "http/response/Response.hpp"
#include "server/Socket.hpp"

class Connection
{
  public:
    // constructor/destructor
    Connection(Socket& conn_socket);
    ~Connection();

    // api
    void handle_event(const epoll_event& on_event);
    bool done() const;
    int fd() const;
    std::vector<EventAction> give_events();

  private:
    enum State
    {
        Receiving = 0,
        Processing,
        Sending,
        Done
    };

    // saves current work state
    State work_state_;

    // context
    Socket& socket_; // owns the socket
    const ServiceConfig& service_;

    // events
    std::vector<EventAction> m_events;

    // http
    RequestParser parser_;
    RequestProcessor processor_;
    Response response_;

    // illegal - no copying of connections is allowed
    // every connection must be a reference/pointer
    // to a connection from <<<ConnectionPool>>>
    Connection(const Connection&);
    Connection& operator=(const Connection&);
};

#endif /* CONNECTION_HPP */
