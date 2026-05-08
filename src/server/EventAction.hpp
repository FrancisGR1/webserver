#ifndef EVENT_ACTION_HPP
#define EVENT_ACTION_HPP

#include <string>

#include <stdint.h>
#include <sys/epoll.h>

class Connection;

struct EventAction
{
    enum Action
    {
        // send to event manager
        WantRead = 0,
        WantProcessRequest,
        WantWrite,
        WantClose
    };

    enum Type
    {
        ServerSocket = 0,
        ClientSocket,
        Pipe,
    };

    // construct/copy
    EventAction(EventAction::Action action, EventAction::Type type, int fd, Connection* conn);
    EventAction(const EventAction& other);

    Action action;
    Type type;
    int fd;
    Connection* conn;

    std::string str() const;
};

typedef EventAction EA;

// overload
bool operator==(const EventAction& event_action, const epoll_event& epoll_event);

#endif // EVENT_ACTION_HPP
