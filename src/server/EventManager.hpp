#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include <list>
#include <set>
#include <vector>

#include "server/ConnectionPool.hpp"
#include "server/EventAction.hpp"

class ConnectionPool;

class EventManager
{
  public:
    // construct/destruct
    EventManager(size_t max_events, ConnectionPool& connections);
    ~EventManager();

    // manage events
    int wait();
    bool next(EventAction*& out);
    int apply(const std::vector<EventAction>& event_actions);
    int apply(const EventAction& ea);

  private:
    // data
    //-events
    int m_epoll_fd;
    // an event is one of the following: socket, file, pipe
    epoll_event* m_epoll_events_buffer;
    std::list<EventAction*> m_events;
    std::list<EventAction*> m_pending_deletes;
    //-iteration
    std::set<int> m_processed_fds;
    size_t m_current_event;
    int m_n_events;
    size_t m_max_events;
    //-connections
    ConnectionPool& m_connection_pool;

    // utils
    const EventAction* get_event(size_t index) const;
    int to_epoll_event(const EventAction& ea);
    EventAction* add(const EventAction& ea);
    int remove(const EventAction& ea);
    void modify(EventAction*& ea, EventAction::Action action);
    EventAction* find(EventAction*& ee) const;
    EventAction* exists(int event_fd) const;
    void flush_pending_deletes(void);
};

#endif /* EVENT_MANAGER */
