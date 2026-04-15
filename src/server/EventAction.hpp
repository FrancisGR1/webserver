#ifndef EVENT_ACTION_HPP
#define EVENT_ACTION_HPP

#include <stdint.h>
#include <sys/epoll.h>

#include <cstddef>

class Connection;

struct EventAction
{
    enum Action
    {
        // send to event manager
        WantReading = 0,
        WantWriting,
        WantClose,
        WantNothing
    };

    EventAction(EventAction::Action action, int fd, Connection* conn = NULL);
    EventAction(const epoll_event& ev);

    Action action;
    int fd;
    Connection* conn;
};

#endif // EVENT_ACTION_HPP
