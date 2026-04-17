#include <cstring>

#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "http/processor/RequestProcessor.hpp"
#include "http/request/Request.hpp"
#include "server/Connection.hpp"
#include "server/EventAction.hpp"
#include "server/Socket.hpp"
#include "server/Webserver.hpp"

Connection::Connection(int client_fd, const Listener& listener, const ServiceConfig& service)
    : m_state(Receiving)
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

// @QUESTION é possível haver um mismatch do evento e do estado?
// por exemplo, a ligação pode estar no estado de escrita e receber um evento de epollin, ou seja, quanddo a ligação
// estiver a enviar, o cliente pde enviar ao mesmo tempo? tudo o que está abaixo assume que o cliente respeita/dá tempo
// à ligação
void Connection::handle_event(const EventAction& action)
{
    //@TODO: check if event matches connection state
    (void)action;

    switch (m_state)
    {
        case Receiving:
        {
            Logger::trace("Connection: state - Receiving");

            // read client info to buffer
            char buffer[constants::read_chunk_size + 1];
            ssize_t bytes = recv(m_socket.fd(), buffer, constants::read_chunk_size, 0);
            if (bytes == 0)
            {
                Logger::info("Connection: Client closed connection on fd %d", m_socket.fd());
                m_state = Done;
                return;
            }
            if (bytes == -1)
            {
                Logger::warn("Connnection: recv() returned -1: errno says: '%s'", strerror(errno));
                //@TODO substituir por timeout, se timeout = yes, então done, caso contrário não
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // Trace instead of Warn: this is a normal part of non-blocking I/O
                    Logger::trace("Connection: recv() EAGAIN on fd %d (waiting for data)", m_socket.fd());
                    return;
                }
                Logger::error("Connection: fatal recv() error on fd %d: %s", m_socket.fd(), strerror(errno));
                m_state = Done; // Only set Done on ACTUAL errors
                return;
            }

            // parser works on raw strings but just to be safe...
            buffer[bytes] = '\0';

            // parse
            m_parser.feed(buffer);

            //@TODO juntar isto à fase de sending
            // sending -> processing
            // o evento deve ser mudado para EPOLLOUT na altura de
            // escrever a resposta e não como está agora
            // (na altura de GET/POST de ficheiro)
            if (m_parser.done())
            {
                const Request& request_ = m_parser.get();
                Logger::debug_obj(request_, "Connection: Request: ");
                m_processor.set(request_);
                m_events.push_back(
                    EventAction(EventAction::WantProcessing, EventAction::ClientSocket, m_socket.fd(), this));
                m_state = Processing;
            }
            break;
        }
        case Processing:
        {
            Logger::trace("Connection: state - Processing");
            m_processor.process();

            if (m_processor.done())
            {
                Logger::trace("Connection: RequestProcessor: Done!");
                m_response = m_processor.response();
                Logger::debug_obj(m_response, "Connection: Response:\n");
                m_events.push_back(
                    EventAction(EventAction::WantWriting, EventAction::ClientSocket, m_socket.fd(), this));
                m_state = Sending;
            }
            break;
        }
        case Sending:
        {
            Logger::trace("Connection: state - Sending");

            // write to socket
            m_response.send(m_socket.fd());

            if (m_response.done())
            {
                Logger::debug_obj(m_response, "Connection: Response:\n");
                Logger::info("Connection: state - done!");

                // close response body fd
                if (m_response.body_fd() != -1)
                    m_events.push_back(
                        EventAction(EventAction::WantClose, EventAction::LocalFile, m_response.body_fd(), this));
                // close client socket by DEFAULT (http 1.0)
                m_events.push_back(EventAction(EventAction::WantClose, EventAction::ClientSocket, m_socket.fd(), this));

                m_state = Done;
            }
            break;
        }
        default: break;
    }
}

bool Connection::done() const
{
    return m_state == Done;
}

int Connection::fd() const
{
    return m_socket.fd();
}

std::vector<EventAction> Connection::give_events()
{

    // insert processor events before connection events
    std::vector<EventAction> pe = m_processor.give_events();
    m_events.insert(m_events.begin(), pe.begin(), pe.end());

    // drain and return
    std::vector<EventAction> result;
    result.swap(m_events);

    Logger::trace("Connection: give '%zu' events", result.size());

    return result;
}

const ServiceConfig& Connection::service() const
{
    return m_service;
}

const Socket& Connection::socket() const
{
    return m_socket;
}
