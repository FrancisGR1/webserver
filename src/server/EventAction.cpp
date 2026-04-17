#include "server/EventAction.hpp"
#include "core/Logger.hpp"
#include "server/Connection.hpp"

#include <sys/epoll.h>
#include <unistd.h>

EventAction::EventAction(EventAction::Action action, EventAction::Type type, int fd, Connection* conn)
    : action(action)
    , type(type)
    , fd(fd)
    , conn(conn)
{
    Logger::trace("EventAction: constructor - params");
}

EventAction::EventAction(const EventAction& other)
    : action(other.action)
    , type(other.type)
    , fd(other.fd)
    , conn(other.conn)
{
    Logger::trace("EventAction: copy constructor");
}

bool operator==(const EventAction& event_action, const epoll_event& epoll_event)
{
    if (epoll_event.events & EPOLLIN)
    {
        return event_action.action == EventAction::WantReading;
    }
    else if (epoll_event.events & EPOLLOUT)
    {
        return (
            event_action.action == EventAction::WantWriting &&
            (event_action.type == EventAction::ClientSocket || event_action.type == EventAction::LocalFile));
    }
    else
    {
        return event_action.action == EventAction::WantClose;
    }
}
