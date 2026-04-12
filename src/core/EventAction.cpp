#include "core/EventAction.hpp"
#include "server/Connection.hpp"

#include <stdint.h>

EventAction::EventAction(EventAction::Action action, int fd, Connection* conn)
    : action(action)
    , fd(fd)
    , conn(conn)
{
}
