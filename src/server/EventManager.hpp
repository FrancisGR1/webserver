#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include <set>
#include <vector>

#include <sys/epoll.h>
#include <unistd.h>

#include "core/EventAction.hpp"

class EventManager
{
  public:
    // construct/destruct
    EventManager();
    ~EventManager();

    // api
    const epoll_event& get_event(int index) const;
    void apply(const std::vector<EventAction>& actions, Connection* ctx);
    int wait();
    int add(int fd, uint32_t events);
    int remove(int fd);

  private:
    int m_epoll_fd;
    // events are either sockets, files or pipes
    epoll_event m_events[1024];
    std::set<int> m_tracked_fds;

    // utils
    bool is_tracked(int fd);
    void untrack(int fd);
};

#endif /* EVENT_MANAGER */
