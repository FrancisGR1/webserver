#include "EventManager.hpp"
#include "core/Logger.hpp"
#include "core/contracts.hpp"
#include "server/Connection.hpp"
#include "server/EventAction.hpp"

#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>

EventManager::EventManager(size_t max_events)
    : m_epoll_fd(-1)
    , m_max_events(max_events)
    , m_epoll_events_buffer(new epoll_event[max_events])
{
    Logger::trace("EventManager: constructor");

    m_epoll_fd = epoll_create(1);
    if (m_epoll_fd == -1)
    {
        throw std::runtime_error("EventManager: Failed to create epoll fd!");
    }

    m_events.reserve(max_events);
}

EventManager::~EventManager()
{
    Logger::trace("EventManager: destructor");

    if (m_epoll_fd != -1)
        ::close(m_epoll_fd);

    delete[] m_epoll_events_buffer;

    for (size_t i = 0; i < m_events.size(); ++i)
    {
        ::close(m_events[i]->fd);
        delete m_events[i];
    }
}

int EventManager::wait()
{
    Logger::info("EventManager: waiting...");

    int n = epoll_wait(m_epoll_fd, m_epoll_events_buffer, constants::max_events, constants::epoll_timeout);
    if (n == -1)
        Logger::warn("EventManager: epoll_wait() interrupted!");
    return (n);
}

const EventAction& EventManager::get_event(size_t index) const
{
    REQUIRE(index < m_max_events, "Out of bounds!");
    REQUIRE(m_epoll_events_buffer[index].data.ptr != NULL, "data.ptr must never be null!");

    EventAction* ea = static_cast<EventAction*>(m_epoll_events_buffer[index].data.ptr);
    Logger::trace("EventManager: Getting event idx=%d,fd=%d", index, ea->fd);

    return (*find(ea));
}

int EventManager::apply(const EventAction& ea)
{
    int ret = to_epoll_event(ea);
    if (ret == -1)
        return ret;
    return ret;
}

int EventManager::apply(const std::vector<EventAction>& event_actions)
{
    int ret = 0;
    for (size_t i = 0; i < event_actions.size(); ++i)
    {
        ret = apply(event_actions[i]);
    }
    return ret;
}

// utils

// transform EventAction to epoll and save EventAction in m_events
int EventManager::to_epoll_event(const EventAction& ea)
{
    epoll_event ev;
    int ret = 0;
    EventAction* stored_event = exists(ea.fd);

    switch (ea.action)
    {
        case EventAction::WantProcessRequest:
        case EventAction::WantWrite:
        case EventAction::WantRead:
        {
            ev.events = ea.action == EventAction::WantRead ? EPOLLIN : EPOLLOUT;

            if (stored_event != NULL)
            {
                // modify existing event
                modify(stored_event, ea.action);
                ev.data.ptr = stored_event;
                ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, ea.fd, &ev);
            }
            else
            {
                stored_event = add(ea);
                ev.data.ptr = stored_event;
                ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, ea.fd, &ev);
            }

            Logger::trace(
                "EventManager: set fd='%zu' to '%s'",
                ea.fd,
                ea.action == EventAction::WantRead ? "EPOLLIN" : "EPOLLOUT");

            break;
        }

        case EventAction::WantClose:
        {
            ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, ea.fd, NULL);
            remove(ea);

            Logger::trace("EventManager: delete fd='%zu'", ea.fd);

            break;
        }
    }

    if (ret == -1)
    {
        Logger::error("EventManager: epoll_ctl() error: says: '%s'", strerror(errno));
    }

    return ret;
}

void EventManager::remove(const EventAction& ea)
{
    for (size_t i = 0; i < m_events.size(); ++i)
    {
        EventAction* e = m_events[i];

        INVARIANT(e != NULL, "EventAction* in events must never be null!");

        if (ea.fd == e->fd)
        {
            Logger::trace("EventManager: removing connection[id=%lld] fd: '%d'", (e->conn ? e->conn->id() : -1), e->fd);

            ::close(e->fd);
            delete e;
            m_events.erase(m_events.begin() + i);

            return;
        }
    }

    Logger::warn("EventManager: didn't find fd='%d' to remove", ea.fd);
    return;
}

EventAction* EventManager::add(const EventAction& ea)
{
    REQUIRE(ea.fd >= 0, "EventAction fd must be valid: '%d'", ea.fd);
    REQUIRE(!exists(ea.fd), "EventAction fd must not exist: '%d'", ea.fd);

    Logger::trace("EventManager: add fd='%d' to events", ea.fd);
    m_events.push_back(new EventAction(ea));
    return m_events.back();
}

void EventManager::modify(EventAction*& ea, EventAction::Action action)
{
    REQUIRE(ea != NULL, "EventAction must never be null");

    Logger::trace("EventManager: modify fd='%d' to action='%d'", ea->fd, action);
    ea->action = action;
}

//@MUST find a corresponding EventAction or it fails
EventAction* EventManager::find(EventAction*& ee) const
{
    REQUIRE(ee != NULL, "EventAction* must never be NULL!");

    for (size_t i = 0; i < m_events.size(); ++i)
    {
        if (ee == m_events[i])
        {
            INVARIANT(ee == m_events[i], "EventAction must always be equal to epoll_event!");
            if (m_events[i]->conn)
                Logger::trace("EventManager: found connection: '%lld'", m_events[i]->conn->id());
            else
                Logger::trace("EventManager: server socket: '%d'", m_events[i]->fd);
            return m_events[i];
        }
    }

    INVARIANT(false, "Must always find a corresponding 'EventAction*'!");
    return NULL; // unreachable
}

// check if it exists in stored events
EventAction* EventManager::exists(int event_fd) const
{
    for (size_t i = 0; i < m_events.size(); ++i)
    {
        if (event_fd == m_events[i]->fd)
        {
            if (m_events[i]->conn)
                Logger::trace("EventManager: found connection: '%lld'", m_events[i]->conn->id());
            else
                Logger::trace("EventManager: server socket: '%d'", m_events[i]->fd);
            return m_events[i];
        }
    }

    return NULL;
}
