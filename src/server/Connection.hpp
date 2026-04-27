#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>

#include "core/Timer.hpp"
#include "http/processor/RequestProcessor.hpp"
#include "http/request/RequestParser.hpp"
#include "http/response/Response.hpp"
#include "server/EventAction.hpp"
#include "server/Socket.hpp"

// connection state machine
struct ConnectionState
{
    enum Enum
    {
        Reading = 0,
        ProcessingRequest,
        Writing,
        Done
    } state;

    ConnectionState(ConnectionState::Enum state);
    std::string str() const;
};

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
    std::time_t last_activity();
    void send_error(StatusCode::Code code);

    // states
    void read();
    void process_request();
    void write();

    // getters
    ConnectionState::Enum state() const;
    const ServiceConfig& service() const;
    const Socket& socket() const;
    long long id() const;
    const EventAction& event() const;

    // setters
    void set(const EventAction& event);

    // signature
    static long long s_conn_counter;

  private:
    // saves current work state
    ConnectionState m_state;
    Seconds m_last_activity;

    // context
    const long long m_id;
    const ServiceConfig& m_service;
    Socket m_socket; // owns
    const EventAction* m_event_being_handled;

    // events produced
    std::vector<EventAction> m_events;

    // @TODO subtituir por HttpContext
    RequestParser m_parser;
    RequestProcessor m_processor;
    Response m_response;

    // utils
    void next_state(ConnectionState::Enum state);
    void register_event(EventAction event);
    void register_action(EventAction::Action action);
    void update_activity(void);

    // illegal - copy semantics
    // every connection must be a reference/pointer
    // to a connection from <<<ConnectionPool>>>
    // EVERY connection must be constructed in the Pool
    Connection(const Connection&);
    Connection& operator=(const Connection&);
};

#endif /* CONNECTION_HPP */
