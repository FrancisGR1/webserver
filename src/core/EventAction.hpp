#ifndef EVENT_ACTION_HPP
#define EVENT_ACTION_HPP

#include <stdint.h>

#include <cstddef>

class Connection;

struct EventAction
{
    enum Action
    {
        WantReading = 0,
        WantWriting,
        WantClose,
        WantDelete,
        Stay, // do nothing
    };

    EventAction(EventAction::Action action, int fd, Connection* conn = NULL);

    Action action;
    int fd;
    Connection* conn;
};

#endif // EVENT_ACTION_HPP
