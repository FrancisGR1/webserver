#include <cstdlib>

#include "Connection.hpp"
#include "Socket.hpp"
#include "Webserver.hpp"
#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "http/processor/RequestProcessor.hpp"
#include "http/request/Request.hpp"

Connection::Connection(Socket& conn_socket, EventManager& events)
    : work_state_(Receiving)
    , socket_(conn_socket)
    , service_(conn_socket.service())
    , events_(events)
    , processor_(conn_socket, events)
{
    Logger::trace("Connection: constructor");
}

Connection::~Connection()
{
    Logger::trace("Connection: destructor");
}

void Connection::work(const epoll_event& on_event)
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
            if (bytes <= 0)
            {
                //@TODO
                //@QUESTION erro do parser?
                break;
            }

            // parser works on raw strings but just to be safe...
            buffer[bytes] = '\0';

            // parse
            parser_.feed(buffer);

            Logger::debug_obj(parser_.get(), "Connection: Request: ");
            if (parser_.done())
            {
                const Request& request_ = parser_.get();
                processor_.set(request_);
                // events_.modify(socket_.fd(), EPOLLOUT);
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
                work_state_ = Sending;
            }
            break;
        }
        case Sending:
        {
            Logger::trace("Connection: state - Sending");

            response_.send(socket_.fd());

            if (response_.done())
            {
                Logger::debug_obj(response_, "Connection: Response:\n");
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
