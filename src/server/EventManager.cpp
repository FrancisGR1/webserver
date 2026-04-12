#include "EventManager.hpp"
#include "core/EventAction.hpp"
#include "core/Logger.hpp"

#include <cstring>

EventManager::EventManager()
    : m_epoll_fd(-1)
{
    m_epoll_fd = epoll_create(1);
    if (m_epoll_fd == -1)
        Logger::error("EventManager: Failed to create epoll fd!");
}

EventManager::~EventManager()
{
    if (m_epoll_fd != -1)
        close(m_epoll_fd);
}

const epoll_event& EventManager::get_event(int index) const
{
    Logger::trace("EventManager: Getting event [%d]:%d", index, m_events[index].data.fd);
    return (m_events[index]);
}

void EventManager::apply(const std::vector<EventAction>& actions, Connection* ctx)
{
    for (size_t i = 0; i < actions.size(); ++i)
    {
        const EventAction& a = actions[i];
        epoll_event ev;
        ev.data.ptr = ctx;

        switch (a.action)
        {
            case EventAction::WantReading:
                ev.events = EPOLLIN;
                epoll_ctl(m_epoll_fd, is_tracked(a.fd) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, a.fd, &ev);
                break;
            case EventAction::WantWriting:
                ev.events = EPOLLOUT;
                epoll_ctl(m_epoll_fd, is_tracked(a.fd) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, a.fd, &ev);
                break;
            case EventAction::WantClose:
                epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, a.fd, NULL);
                close(a.fd);
                untrack(a.fd);
                break;
            case EventAction::WantDelete:
                epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, a.fd, NULL);
                untrack(a.fd);
                break;
            case EventAction::Stay: // do nothing
                break;
        }
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

bool EventManager::is_tracked(int fd)
{
    return m_tracked_fds.count(fd) > 0;
}

void EventManager::untrack(int fd)
{
    m_tracked_fds.erase(fd);
}
