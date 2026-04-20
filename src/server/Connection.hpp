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
    std::vector<EventAction> give_events();
    int fd() const;
    bool done() const;
    //@TODO implementar
    bool is_keep_alive();
    std::time_t last_activity();

    // states
    void read();
    void process_request();
    void write();
    void close_with(StatusCode::Code code);

    // getters
    const ServiceConfig& service() const;
    const Socket& socket() const;

  private:
    // connection state machine
    enum State
    {
        Reading = 0,
        ProcessingRequest,
        Writing,
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

    // utils
    void next_state(State state);
    void register_event(EventAction event);
    void register_action(EventAction::Action action);

    // illegal - copy semantics
    // every connection must be a reference/pointer
    // to a connection from <<<ConnectionPool>>>
    // EVERY connection must be constructed in the Pool
    Connection(const Connection&);
    Connection& operator=(const Connection&);
};

#endif /* CONNECTION_HPP */
