#include "server/EventAction.hpp"
#include "core/Logger.hpp"
#include "server/Connection.hpp"

#include <sstream>

#include <sys/epoll.h>
#include <unistd.h>

EventAction::EventAction(EventAction::Action action, EventAction::Type type, int fd, Connection* conn)
    : action(action)
    , type(type)
    , fd(fd)
    , conn(conn)
{
    Logger::verbose("EventAction: constructor - params");
    if (conn)
        conn->set(*this);
}

EventAction::EventAction(const EventAction& other)
    : action(other.action)
    , type(other.type)
    , fd(other.fd)
    , conn(other.conn)
{
    Logger::verbose("EventAction: copy constructor");
    if (conn)
        conn->set(*this);
}

std::string EventAction::str() const
{
    std::stringstream ss;

    std::string action_str;
    switch (action)
    {
        case WantRead: action_str = "WantRead"; break;
        case WantProcessRequest: action_str = "WantProcessRequest"; break;
        case WantWrite: action_str = "WantWrite"; break;
        case WantClose: action_str = "WantClose"; break;
    }
    ss << "action='" << action_str << "',";

    std::string type_str;
    switch (type)
    {
        case ServerSocket: type_str = "ServerSocket"; break;
        case ClientSocket: type_str = "ClientSocket"; break;
        case Pipe: type_str = "Pipe"; break;
    }
    ss << "type='" << type_str << "',";

    ss << "fd='" << fd << "',";
    ss << "conn='" << (conn != NULL ? conn->id() : -1);

    return ss.str();
}

bool operator==(const EventAction& event_action, const epoll_event& epoll_event)
{
    if (epoll_event.events & EPOLLIN)
    {
        return event_action.action == EventAction::WantRead;
    }
    else if (epoll_event.events & EPOLLOUT)
    {
        return (
            event_action.action == EventAction::WantWrite &&
            (event_action.type == EventAction::ClientSocket ||
             event_action.type == EventAction::Pipe)); // only client sockets and pipes are writable
    }
    else
    {
        return event_action.action == EventAction::WantClose;
    }
}
