#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include <vector>

#include "server/EventAction.hpp"

class EventManager
{
  public:
    // construct/destruct
    EventManager(size_t max_events);
    ~EventManager();

    // manage events
    int wait();
    const EventAction& get_event(size_t index) const;
    int apply(const std::vector<EventAction>& event_actions);
    int apply(const EventAction& ea);

  private:
    int m_epoll_fd;
    size_t m_max_events;
    // an event is one of the following: socket, file, pipe
    epoll_event* m_epoll_events_buffer;
    std::vector<EventAction*> m_events;

    // utils
    int to_epoll_event(const EventAction& ea);
    EventAction* add(const EventAction& ea);
    int remove(const EventAction& ea);
    void modify(EventAction*& ea, EventAction::Action action);
    EventAction* find(EventAction*& ee) const;
    EventAction* exists(int event_fd) const;
};

#endif /* EVENT_MANAGER */
