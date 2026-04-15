#include "EventManager.hpp"
#include "core/Logger.hpp"
#include "core/contracts.hpp"
#include "server/EventAction.hpp"

#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>

EventManager::EventManager(size_t max_events)
    : m_epoll_fd(-1)
    , m_max_events(max_events)
    , m_events(new epoll_event[max_events])
{
    Logger::trace("EventManager: constructor");

    m_epoll_fd = epoll_create(1);
    if (m_epoll_fd == -1)
    {
        throw std::runtime_error("EventManager: Failed to create epoll fd!");
    }
}

EventManager::~EventManager()
{
    Logger::trace("EventManager: destructor");

    if (m_epoll_fd != -1)
        close(m_epoll_fd);

    delete[] m_events;
}

//@TODO @REFACTOR: should return an event action isntead of an epoll_event
// epoll_events should only live in EventManager
EventAction EventManager::get_event(size_t index) const
{
    REQUIRE(index < m_max_events, "Out of bounds!");

    Logger::trace("EventManager: Getting event [%d]:%d", index, m_events[index].data.fd);

    return (EventAction(m_events[index]));
}

void EventManager::apply(const EventAction& ea)
{
    to_epoll_event(ea, ea.conn);
}

void EventManager::apply(const std::vector<EventAction>& actions, Connection* ctx)
{
    for (size_t i = 0; i < actions.size(); ++i)
    {
        const EventAction& ea = actions[i];
        to_epoll_event(ea, ctx);
    }
}

int EventManager::wait()
{
    int n = epoll_wait(m_epoll_fd, m_events, constants::max_events, constants::epoll_timeout);
    if (n == -1)
        Logger::error("Failed to wait on epoll!");
    return (n);
}

int EventManager::add(int fd, uint32_t events)
{
    epoll_event event = {};
    event.events = events;
    event.data.fd = fd;
    int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1)
        Logger::error("epoll_ctl() ADD failed!: tried to add fd=%d", fd);
    return ret;
}

int EventManager::remove(int socket)
{
    Logger::trace("EventManager: Remove fd %d from events", socket);
    int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, socket, NULL);
    if (ret == -1)
        Logger::error("epoll_ctl() DEL failed!: tried to delete fd=%d", socket);
    return ret;
}

// utils
bool EventManager::is_tracked(int fd)
{
    return m_tracked_fds.count(fd) > 0;
}

void EventManager::untrack(int fd)
{
    m_tracked_fds.erase(fd);
}
void EventManager::to_epoll_event(const EventAction& ea, Connection* ctx)
{
    epoll_event ev;
    ev.data.ptr = ctx;

    switch (ea.action)
    {
            //@TODO apanhar possível error de epoll_ctl
        case EventAction::WantReading:
            ev.events = EPOLLIN;
            epoll_ctl(m_epoll_fd, is_tracked(ea.fd) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, ea.fd, &ev);
            break;
        case EventAction::WantWriting:
            ev.events = EPOLLOUT;
            epoll_ctl(m_epoll_fd, is_tracked(ea.fd) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, ea.fd, &ev);
            break;
        case EventAction::WantClose:
            epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, ea.fd, NULL);
            close(ea.fd); //@WARN @ERROR é suposto fechar aqui?
            untrack(ea.fd);
            break;
        case EventAction::WantNothing: // do nothing
            break;
    }
}
