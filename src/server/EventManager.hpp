#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include <sys/epoll.h>
#include <iostream>
#include <unistd.h>

class EventManager
{
	private:
		int			epoll_fd_;
		epoll_event	events_[1024];

	public:
		EventManager();
		~EventManager();

		epoll_event&	getEvent(int index);
		int				wait();
		
		void			add(const int socket, uint32_t events);
		void			modify(int socket, uint32_t events);
		void			remove(int socket);
		
		void			log(const std::string &message);
	
};

#endif /* EVENT_MANAGER */