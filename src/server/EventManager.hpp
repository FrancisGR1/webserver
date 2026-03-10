#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include <sys/epoll.h>
#include <unistd.h>

class EventManager
{
	public:
		EventManager();
		~EventManager();

		epoll_event&	getEvent(int index);
		int				wait();
		
		int			add(int socket, uint32_t events);
		int			modify(int socket, uint32_t events);
		int			remove(int socket);

	private:
		int			epoll_fd_;
		// events are either sockets, files or pipes
		epoll_event	events_[1024];

	
};

#endif /* EVENT_MANAGER */
