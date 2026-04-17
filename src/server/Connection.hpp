#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>

#include "http/processor/RequestProcessor.hpp"
#include "http/request/RequestParser.hpp"
#include "http/response/Response.hpp"
#include "server/EventAction.hpp"
#include "server/Socket.hpp"

//@TODO adicionar um unique id
class Connection
{
  public:
    // constructor/destructor
    Connection(int client_fd, const Listener& listener, const ServiceConfig& service);
    ~Connection();

    // api
    void handle_event(const EventAction& action);
    std::vector<EventAction> give_events();
    int fd() const;
    bool done() const;
    //@TODO implementar
    bool is_keep_alive();
    std::time_t last_activity();

    // getters
    const ServiceConfig& service() const;
    const Socket& socket() const;

  private:
    // connection state machine
    enum State
    {
        Receiving = 0,
        Processing,
        Sending,
        Done
    };

    // saves current work state
    State m_state;

    // context
    const ServiceConfig& m_service;
    Socket m_socket; // owns

    // events produced
    std::vector<EventAction> m_events;

    // @TODO subtituir por HttpContext
    RequestParser m_parser;
    RequestProcessor m_processor;
    Response m_response;

    // illegal - copy semantics
    // every connection must be a reference/pointer
    // to a connection from <<<ConnectionPool>>>
    // and must be constructed from there
    Connection(const Connection&);
    Connection& operator=(const Connection&);
};

#endif /* CONNECTION_HPP */
