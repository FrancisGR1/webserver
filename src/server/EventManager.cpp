#include "EventManager.hpp"

EventManager::EventManager()
{
	epoll_fd_ = epoll_create(1);
	if (epoll_fd_ == -1)
		log("Error: Failed to create epoll fd!");
}

EventManager::~EventManager()
{
	if (epoll_fd_ != -1)
		close(epoll_fd_);
}

epoll_event&	EventManager::getEvent(int index)
{
	return (events_[index]);
}

/* member fuctions */

void	EventManager::log(const std::string &message)
{
	std::cout << message << std::endl;
}

int	EventManager::wait()
{
	int n = epoll_wait(epoll_fd_, events_, 1024, -1);
	if (n == -1)
		log("Erro: Failed to wait on epoll!");
	return (n);
}

void	EventManager::add(const int socket, uint32_t events)
{
	epoll_event event = {};
	event.events = events;
	event.data.fd = socket;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket, &event) == -1)
		log("Error: epoll_ctl ADD failed!");
}

void	EventManager::modify(int socket, uint32_t events)
{
	epoll_event event = {};
	event.events = events;
	event.data.fd = socket;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket, &event) == -1)
		log("Error: epoll_ctl MOD failed!");
}

void	EventManager::remove(int socket)
{
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket, NULL) == -1)
		log("Error: epoll_ctl DEL failed!");
}



