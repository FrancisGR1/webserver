#include <cstring>

#include "core/Logger.hpp"
#include "core/Timer.hpp"
#include "core/constants.hpp"
#include "core/contracts.hpp"
#include "http/processor/RequestProcessor.hpp"
#include "server/Connection.hpp"
#include "server/EventAction.hpp"
#include "server/Socket.hpp"
#include "server/Webserver.hpp"

ConnectionState::ConnectionState(ConnectionState::Enum state)
    : state(state)
{
}

std::string ConnectionState::str() const
{
    switch (state)
    {
        case Reading: return "Reading";
        case ProcessingRequest: return "ProcessingRequest";
        case Writing: return "Writing";
        case Done: return "Done";
    }

    return "";
}

static long long s_connection_counter = 0;

Connection::Connection(int client_fd, const Listener& listener, const ServiceConfig& service)
    : m_state(ConnectionState::Reading)
    , m_last_activity(Timer::now())
    , m_id(++s_connection_counter)
    , m_service(service)
    , m_socket(client_fd, listener)
    , m_processor(this, service)
{
    Logger::trace("%s[id=%lld]: constructor", constants::conn, m_id);
}

Connection::~Connection()
{
    Logger::trace("%s[id=%lld]: destructor", constants::conn, m_id);
}

std::vector<EventAction> Connection::give_events()
{

    // drain and return
    std::vector<EventAction> result;
    result.swap(m_events);

    Logger::trace("%s[id=%lld]: give '%zu' events", constants::conn, m_id, result.size());

    return result;
}

int Connection::fd() const
{
    return m_socket.fd();
}

long long Connection::id() const
{
    return m_id;
}

bool Connection::done() const
{
    return m_state.state == ConnectionState::Done;
}

std::time_t Connection::last_activity()
{
    return m_last_activity;
}

void Connection::read()
{
    REQUIRE(m_state.state == ConnectionState::Reading);
    if (m_state.state != ConnectionState::Reading)
    {
        Logger::warn("%s[id=%lld]: Trying to read when in '%s' state", constants::conn, m_id, m_state.str().c_str());
    }

    Logger::trace("%s[id=%lld]: state - Reading", constants::conn, m_id);

    // read client info to buffer
    char buffer[constants::read_chunk_size + 1];
    ssize_t bytes = recv(m_socket.fd(), buffer, constants::read_chunk_size, 0);
    if (bytes == 0)
    {
        Logger::info("%s[id=%lld]: Client closed connection on fd %d", constants::conn, m_id, m_socket.fd());
        next_state(ConnectionState::Done);
        return;
    }
    else if (bytes == -1)
    {
        Logger::warn("Connnection: recv() returned -1: errno: '%s'", ::strerror(errno));

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Trace instead of Warn: this is a normal part of non-blocking I/O
            Logger::trace(
                "%s[id=%lld]: recv() error on fd %d (waiting for data): error says: '%s'",
                constants::conn,
                m_id,
                m_socket.fd(),
                ::strerror(errno));
        }
        else
        {
            Logger::error(
                "%s[id=%lld]: recv() error on fd %d: '%s'", constants::conn, m_id, m_socket.fd(), ::strerror(errno));
        }

        return;
    }
    else
    {
        Logger::debug("Connection[id=%lld]: read %ld bytes from client", m_id, bytes);
    }

    // parse
    m_parser.feed(std::string(buffer, bytes));

    if (m_parser.done())
    {
        const Request& request_ = m_parser.get();
        Logger::debug_obj(request_, "Connection: Request: ");
        m_processor.set(request_);
        next_state(ConnectionState::ProcessingRequest);

        process_request(); // try fast path -> process immediately

        if (!m_processor.done()) // slow path -> stay in processing state
        {
            std::vector<EventAction> events = m_processor.give_events();
            if (m_processor.is_cgi()) // register cgi pipes
            {
                for (size_t i = 0; i < events.size(); ++i)
                    register_event(events[i]);
            }
            else // default to epollout
            {
                register_action(EventAction::WantProcessRequest);
            }
        }
        else // fast path successful -> bypass processing state
        {
            Logger::trace("%s[id=%lld]: on fast path", constants::conn, m_id);
        }
    }
    update_activity();
}

void Connection::process_request()
{
    REQUIRE(m_state.state == ConnectionState::ProcessingRequest);
    if (m_state.state != ConnectionState::ProcessingRequest)
    {
        Logger::warn(
            "%s[id=%lld]: Trying to process request when in '%s' state", constants::conn, m_id, m_state.str().c_str());
        return;
    }

    Logger::trace("%s[id=%lld]: state - Processing Request", constants::conn, m_id);

    m_processor.process();

    // drain processor events
    std::vector<EventAction> events = m_processor.give_events();
    for (size_t i = 0; i < events.size(); ++i)
        register_event(events[i]);

    if (m_processor.done())
    {
        Logger::trace("%s[id=%lld]: RequestProcessor: Done!", constants::conn, m_id);
        m_response = m_processor.response();
        Logger::debug_obj(m_response, "Connection: Response:\n");
        register_action(EventAction::WantWrite);
        next_state(ConnectionState::Writing);
    }
    update_activity();
}

void Connection::write()
{
    REQUIRE(m_state.state == ConnectionState::Writing);
    if (m_state.state != ConnectionState::Writing)
    {
        Logger::warn("%s[id=%lld]: Trying to write when in '%s' state", constants::conn, m_id, m_state.str().c_str());
    }

    Logger::trace("%s[id=%lld]: state - Writing", constants::conn, m_id);

    // write to socket
    m_response.send(m_socket.fd());

    if (m_response.done())
    {
        Logger::debug_obj(m_response, "Connection: Response:\n");
        Logger::info("%s[id=%lld]: state - done!", constants::conn, m_id);

        // close client socket by DEFAULT (HTTP 1.0)
        register_action(EventAction::WantClose);
        next_state(ConnectionState::Done);
    }
    update_activity();
}

void Connection::send_error(StatusCode::Code code)
{
    Response res(code);
    res.send(m_socket.fd());
    next_state(ConnectionState::Done);
}

ConnectionState::Enum Connection::state() const
{
    return m_state.state;
}

const ServiceConfig& Connection::service() const
{
    return m_service;
}

const Socket& Connection::socket() const
{
    return m_socket;
}

// utils
void Connection::next_state(ConnectionState::Enum state)
{
    m_state.state = state;
}

void Connection::register_event(EventAction event)
{
    Logger::trace("%s[id=%lld]: register event: '%s'", constants::conn, m_id, event.str().c_str());
    m_events.push_back(event);
}

void Connection::register_action(EventAction::Action action)
{
    EventAction event(action, EventAction::ClientSocket, m_socket.fd(), this);
    register_event(event);
}

void Connection::update_activity(void)
{
    m_last_activity = Timer::now();
}
