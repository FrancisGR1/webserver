#include <cstdlib>
#include <cstring>

#include "Connection.hpp"
#include "Socket.hpp"
#include "Webserver.hpp"
#include "core/EventAction.hpp"
#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "http/processor/RequestProcessor.hpp"
#include "http/request/Request.hpp"

Connection::Connection(Socket& conn_socket)
    : work_state_(Receiving)
    , socket_(conn_socket)
    , service_(conn_socket.service())
    , processor_(conn_socket)
{
    Logger::trace("Connection: constructor");
}

Connection::~Connection()
{
    Logger::trace("Connection: destructor");
}

void Connection::handle_event(const epoll_event& on_event)
{
    //@TODO: check if event matches connection state
    (void)on_event;

    switch (work_state_)
    {
        case Receiving:
        {
            Logger::trace("Connection: state - Receiving");

            // read client info to buffer
            char buffer[constants::read_chunk_size + 1];
            ssize_t bytes = recv(socket_.fd(), buffer, constants::read_chunk_size, 0);
            if (bytes == 0)
            {
                Logger::info("Connection: Client closed connection on fd %d", socket_.fd());
                work_state_ = Done;
                return;
            }
            if (bytes == -1)
            {
                Logger::warn("Connnection: recv() returned -1: errno says: '%s'", strerror(errno));
                //@TODO substituir por timeout, se timeout = yes, então done, caso contrário não
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // Trace instead of Warn: this is a normal part of non-blocking I/O
                    Logger::trace("Connection: recv() EAGAIN on fd %d (waiting for data)", socket_.fd());
                    return;
                }
                Logger::error("Connection: fatal recv() error on fd %d: %s", socket_.fd(), strerror(errno));
                work_state_ = Done; // Only set Done on ACTUAL errors
                return;
            }

            // parser works on raw strings but just to be safe...
            buffer[bytes] = '\0';

            // parse
            parser_.feed(buffer);

            if (parser_.done())
            {
                const Request& request_ = parser_.get();
                Logger::debug_obj(request_, "Connection: Request: ");
                processor_.set(request_);
                work_state_ = Processing;
            }
            // std::exit(1);
            break;
        }
        case Processing:
        {
            processor_.process();

            if (processor_.done())
            {
                Logger::trace("Connection: RequestProcessor: Done!");
                response_ = processor_.response();
                Logger::debug_obj(response_, "Connection: Response:\n");
                m_events.push_back(EventAction(EventAction::WantWriting, socket_.fd()));
                work_state_ = Sending;
            }
            break;
        }
        case Sending:
        {
            Logger::trace("Connection: state - Sending");

            // modificar
            response_.send(socket_.fd());

            if (response_.done())
            {
                Logger::debug_obj(response_, "Connection: Response:\n");
                Logger::info("Connection: state - done!");
                work_state_ = Done;
                //@TODO cleanup de quê?
            }
            break;
        }
        default: break;
    }
}

bool Connection::done() const
{
    return work_state_ == Done;
}

int Connection::fd() const
{
    return socket_.fd();
}

std::vector<EventAction> Connection::give_events()
{
    // insert processor events before connection events
    std::vector<EventAction> pe = processor_.give_events();
    m_events.insert(m_events.begin(), pe.begin(), pe.end());

    // drain and return
    std::vector<EventAction> result;
    result.swap(m_events);
    return result;
}
