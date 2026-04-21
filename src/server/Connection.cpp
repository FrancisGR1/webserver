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

Connection::Connection(int client_fd, const Listener& listener, const ServiceConfig& service)
    : m_state(Reading)
    , m_service(service)
    , m_socket(client_fd, listener)
    , m_processor(this, service)
{
    Logger::trace("Connection: constructor");
}

Connection::~Connection()
{
    Logger::trace("Connection: destructor");
}

std::vector<EventAction> Connection::give_events()
{

    // drain and return
    std::vector<EventAction> result;
    result.swap(m_events);

    Logger::trace("Connection: give '%zu' events", result.size());

    return result;
}

int Connection::fd() const
{
    return m_socket.fd();
}

bool Connection::done() const
{
    return m_state == Done;
}

std::time_t Connection::last_activity()
{
    return m_last_activity;
}

void Connection::read()
{
    REQUIRE(m_state == Reading);

    Logger::trace("Connection: state - Reading");

    // read client info to buffer
    char buffer[constants::read_chunk_size + 1];
    ssize_t bytes = recv(m_socket.fd(), buffer, constants::read_chunk_size, 0);
    if (bytes == 0)
    {
        Logger::info("Connection: Client closed connection on fd %d", m_socket.fd());
        next_state(Done);
        return;
    }
    if (bytes == -1)
    {
        Logger::warn("Connnection: recv() returned -1: errno says: '%s'", strerror(errno));

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Trace instead of Warn: this is a normal part of non-blocking I/O
            Logger::trace(
                "Connection: recv() error on fd %d (waiting for data): error says: '%s'",
                m_socket.fd(),
                strerror(errno));
        }
        else
        {
            Logger::error("Connection: recv() error on fd %d: '%s'", m_socket.fd(), strerror(errno));
        }

        return;
    }

    // parser works on raw strings but just to be safe...
    buffer[bytes] = '\0';

    // parse
    m_parser.feed(buffer);

    if (m_parser.done())
    {
        const Request& request_ = m_parser.get();
        Logger::debug_obj(request_, "Connection: Request: ");
        m_processor.set(request_);
        next_state(ProcessingRequest);

        // try fast path -> process immediately
        process_request();
        if (!m_processor.done())
        // slow path -> stay in processing state
        {
            std::vector<EventAction> events = m_processor.give_events();
            if (!events.empty() > 0)
            // register cgi pipes
            {
                for (size_t i = 0; i < events.size(); ++i)
                    register_event(events[i]);
            }
            else
            // default to epollout
            {
                register_action(EventAction::WantProcessRequest);
            }
        }
        else
        // fast path successful -> bypass processing state
        {
            Logger::trace("Connection: on fast path");
        }
    }
    update_activity();
}

void Connection::process_request()
{
    REQUIRE(m_state == ProcessingRequest);

    Logger::trace("Connection: state - Processing Request");

    m_processor.process();

    if (m_processor.done())
    {
        Logger::trace("Connection: RequestProcessor: Done!");
        m_response = m_processor.response();
        Logger::debug_obj(m_response, "Connection: Response:\n");
        register_action(EventAction::WantWrite);
        next_state(Writing);
    }
    update_activity();
}

void Connection::write()
{
    REQUIRE(m_state == Writing);

    Logger::trace("Connection: state - Writing");

    // write to socket
    m_response.send(m_socket.fd());

    if (m_response.done())
    {
        Logger::debug_obj(m_response, "Connection: Response:\n");
        Logger::info("Connection: state - done!");

        // close client socket by DEFAULT (http 1.0)
        register_action(EventAction::WantClose);
        next_state(Done);
    }
    update_activity();
}

void Connection::send_error(StatusCode::Code code)
{
    Response res(code);
    res.send(m_socket.fd());
    next_state(Done);
}

ConnectionState Connection::state() const
{
    return m_state;
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
void Connection::next_state(ConnectionState state)
{
    m_state = state;
}

void Connection::register_event(EventAction event)
{
    m_events.push_back(event);
}

void Connection::register_action(EventAction::Action action)
{
    m_events.push_back(EventAction(action, EventAction::ClientSocket, m_socket.fd(), this));
}

void Connection::update_activity(void)
{
    m_last_activity = Timer::now();
}
