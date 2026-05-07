#include "server/EventManager.hpp"
#include "core/Logger.hpp"
#include "core/contracts.hpp"
#include "server/Connection.hpp"
#include "server/ConnectionPool.hpp"
#include "server/EventAction.hpp"

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>

EventManager::EventManager(size_t max_events, ConnectionPool& connections)
    : m_epoll_fd(-1)
    , m_epoll_events_buffer(new epoll_event[max_events])
    , m_current_event(0)
    , m_n_events(-1)
    , m_max_events(max_events)
    , m_connection_pool(connections)

{
    Logger::verbose("EventManager: constructor");

    m_epoll_fd = epoll_create(1);
    if (m_epoll_fd == -1)
    {
        throw std::runtime_error("EventManager: Failed to create epoll fd!");
    }
    ::fcntl(m_epoll_fd, F_SETFD, FD_CLOEXEC);
}

EventManager::~EventManager()
{
    Logger::trace("EventManager: destructor");

    if (m_epoll_fd != -1)
        ::close(m_epoll_fd);

    delete[] m_epoll_events_buffer;

    for (std::list<EventAction*>::iterator it = m_events.begin(); it != m_events.end(); ++it)
    {
        ::close((*it)->fd);
        delete *it;
    }
}

int EventManager::wait()
{
    m_n_events = epoll_wait(m_epoll_fd, m_epoll_events_buffer, constants::max_events, constants::epoll_timeout);
    if (m_n_events == -1)
        Logger::warn("EventManager: epoll_wait() interrupted!");
    else if (m_n_events > 0)
    {
        Logger::debug("EventManager: %d event(s) ready:", m_n_events);
        if (Logger::level() <= Log::Debug)
        {
            Logger::debug("==================================");
            for (size_t i = 0; i < static_cast<size_t>(m_n_events); ++i)
            {
                EventAction* ea = static_cast<EventAction*>(m_epoll_events_buffer[i].data.ptr);
                EventAction* found = find(ea);
                if (found)
                    Logger::debug("EventManager: event=%d: '%s'", i, found->str().c_str());
                else
                    Logger::debug("EventManager: event=%d: stale", i);
            }
            Logger::debug("==================================");
        }
    }
    else
    {
        Logger::debug("EventManager: woke up");
    }
    return (m_n_events);
}

bool EventManager::next(EventAction*& out)
{
    if (m_n_events <= 0)
        return false;

    while (m_current_event < static_cast<size_t>(m_n_events))
    {
        EventAction* ea = static_cast<EventAction*>(m_epoll_events_buffer[m_current_event++].data.ptr);

        EventAction* found = find(ea);
        if (found == NULL)
        {
            // skip stale events
            continue;
        }
        out = found;
        return true;
    }

    flush_pending_deletes();

    // reset
    m_current_event = 0;
    m_n_events = 0;

    return false;
}

const EventAction* EventManager::get_event(size_t index) const
{
    REQUIRE(index < m_max_events, "Out of bounds!");
    REQUIRE(m_epoll_events_buffer[index].data.ptr != NULL, "data.ptr must never be null!");

    EventAction* ea = static_cast<EventAction*>(m_epoll_events_buffer[index].data.ptr);
    if (ea == NULL)
    {
        Logger::warn("EventManager: found stale event");
        return NULL;
    }
    else
    {
        Logger::debug("EventManager: Getting idx=%d: '%s'", index, ea->str().c_str());
    }

    return (find(ea));
}

int EventManager::apply(const EventAction& ea)
{
    Logger::debug("EventManager: apply: '%s'", ea.str().c_str());

    int ret = to_epoll_event(ea);
    if (ret == -1)
        return ret;
    return ret;
}

int EventManager::apply(const std::vector<EventAction>& event_actions)
{
    Logger::debug("EventManager: apply %d events", event_actions.size());

    int ret = 0;
    for (size_t i = 0; i < event_actions.size(); ++i)
    {
        ret = apply(event_actions[i]);
    }

    m_connection_pool.remove_idles(*this);

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
            ret = remove(ea);

            break;
        }
    }

    if (ret == -1)
    {
        Logger::error("EventManager: epoll_ctl() error: says: '%s'", strerror(errno));
    }

    return ret;
}

int EventManager::remove(const EventAction& ea)
{
    Logger::trace("EventManager: remove: '%s'", ea.str().c_str());

    int ret = 0;

    for (std::list<EventAction*>::iterator it = m_events.begin(); it != m_events.end(); ++it)
    {
        EventAction* e = *it;

        INVARIANT(e != NULL, "EventAction* in events must never be null!");

        if (ea.fd == e->fd)
        {
            ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, ea.fd, NULL);

            if (ret == -1)
                Logger::error("epoll_ctl DEL failed: %s", strerror(errno));
            if (e->type == EventAction::ClientSocket || e->type == EventAction::ServerSocket)
            {
                ret = ::close(e->fd);
                if (e->type == EventAction::ClientSocket && e->conn != NULL)
                {
                    Logger::trace("EventManager: delete connection[id=%zu] fd='%zu'", ea.conn->id(), ea.fd);
                    m_connection_pool.remove(*e->conn);
                }
                else
                {
                    Logger::trace("EventManager: delete server socket fd='%zu'", ea.fd);
                }
            }
            else
            {
                ret = ::close(e->fd);
                Logger::trace("EventManager: delete pipe fd='%zu'", ea.fd);
            }

            m_pending_deletes.push_back(e);
            m_events.erase(it);

            return ret;
        }
    }

    Logger::warn("EventManager: didn't find fd='%d' to remove", ea.fd);
    return ret;
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

EventAction* EventManager::find(EventAction*& ea) const
{
    if (ea == NULL)
    {
        Logger::warn("EventManager: EventAction is null");
        return NULL;
    }

    for (std::list<EventAction*>::const_iterator it = m_events.begin(); it != m_events.end(); ++it)
    {
        if (ea == *it)
        {
            Logger::trace("EventManager: found fd='%d' in events", ea->fd);
            if ((*it)->conn)
                Logger::trace("EventManager: found connection: '%lld'", (*it)->conn->id());
            else
                Logger::trace("EventManager: server socket: '%d'", (*it)->fd);
            return *it;
        }
    }

    Logger::warn("EventManager: stale event, skipping");
    return NULL;
}

// check if it exists in stored events
EventAction* EventManager::exists(int event_fd) const
{
    REQUIRE(event_fd >= 0);

    for (std::list<EventAction*>::const_iterator it = m_events.begin(); it != m_events.end(); ++it)
    {
        if (event_fd == (*it)->fd)
        {
            if ((*it)->conn)
                Logger::trace("EventManager: found connection: id=%lld,fd=%d", (*it)->conn->id(), (*it)->fd);
            else
                Logger::trace("EventManager: server socket: '%d'", (*it)->fd);
            return *it;
        }
    }

    return NULL;
}

void EventManager::flush_pending_deletes(void)
{
    for (std::list<EventAction*>::iterator it = m_pending_deletes.begin(); it != m_pending_deletes.end(); ++it)
        delete *it;
    m_pending_deletes.clear();
}
