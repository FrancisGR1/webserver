#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include <set>
#include <vector>

#include "server/EventAction.hpp"

class EventManager
{
  public:
    // construct/destruct
    EventManager(size_t max_events);
    ~EventManager();

    // api
    EventAction get_event(size_t index) const;
    void apply(const std::vector<EventAction>& actions, Connection* ctx);
    void apply(const EventAction& actions);
    int wait();
    int add(int fd, uint32_t events);
    int remove(int fd);

  private:
    int m_epoll_fd;
    // an event is one of the following: socket, file, pipe
    size_t m_max_events;
    epoll_event* m_events;
    std::set<int> m_tracked_fds;

    // utils
    bool is_tracked(int fd);
    void untrack(int fd);
    void to_epoll_event(const EventAction& ea, Connection* ctx);
};

#endif /* EVENT_MANAGER */
