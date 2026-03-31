#include <cstring>

#include "EventManager.hpp"
#include "core/Logger.hpp"

EventManager::EventManager()
{
    epoll_fd_ = epoll_create(1);
    if (epoll_fd_ == -1)
        Logger::error("EventManager: Failed to create epoll fd!");
}

EventManager::~EventManager()
{
    if (epoll_fd_ != -1)
        close(epoll_fd_);
}

epoll_event& EventManager::getEvent(int index)
{
    Logger::trace("EventManager: Getting event [%d]:%d", index, events_[index].data.fd);
    return (events_[index]);
}

/* member fuctions */

int EventManager::wait()
{
    int n = epoll_wait(epoll_fd_, events_, 1024, -1);
    if (n == -1)
        Logger::error("Failed to wait on epoll!");
    return (n);
}

int EventManager::add(int socket, uint32_t events)
{
    epoll_event event = {};
    event.events = events;
    // @ASSUMPTION: fds are only sockets
    event.data.fd = socket;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket, &event);
    if (ret == -1)
        Logger::error("epoll_ctl() ADD failed!: tried to add fd=%d", socket);
    return ret;
}

int EventManager::modify(int socket, uint32_t events)
{
    epoll_event event = {};
    event.events = events;
    event.data.fd = socket;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket, &event);
    if (ret == -1)
        Logger::error("epoll_ctl() MOD failed!: tried to modify fd=%d", socket);
    return ret;
}

int EventManager::remove(int socket)
{
    Logger::trace("EventManager: Remove fd %d from events", socket);
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket, NULL);
    if (ret == -1)
        Logger::error("epoll_ctl() DEL failed!: tried to delete fd=%d", socket);
    return ret;
}
