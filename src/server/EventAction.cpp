#include "server/EventAction.hpp"
#include "core/Logger.hpp"
#include "server/Connection.hpp"

#include <stdint.h>
#include <sys/epoll.h>

EventAction::EventAction(EventAction::Action action, int fd, Connection* conn)
    : action(action)
    , fd(fd)
    , conn(conn)
{
    Logger::trace("EventAction: constructor - params");
}

EventAction::EventAction(const epoll_event& ev)
    : action(EventAction::WantClose) // WantClose is default
    , fd(ev.data.fd)
    , conn(static_cast<Connection*>(ev.data.ptr))
{
    Logger::trace("EventAction: constructor - epoll_event");

    // translate epoll_event to EventAction::Action
    if (ev.events & EPOLLIN)
    {
        action = EventAction::WantReading;
    }
    else if (ev.events & EPOLLOUT)
    {
        action = EventAction::WantReading;
    }
    else if (ev.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        action = EventAction::WantClose;
    }
    else
    {
        Logger::warn("EventAction: unknown epoll event: '%zu'. Default to WantClose", ev.events);
        action = EventAction::WantClose;
    }
}
