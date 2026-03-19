#ifndef RESOURCEREGISTRY_HPP
# define RESOURCEREGISTRY_HPP

#include "ConnectionPool.hpp"
#include "EventManager.hpp"

class ResourceRegistry
{
	public:
		ResourceRegistry(ConnectionPool& connection_pool_, EventManager& events_);
		void set(int fd, Connection& conn, uint32_t events);
		void modify(int fd, Connection& conn, uint32_t events);

	private:
		ConnectionPool& connection_pool_;
		EventManager& events_;
};

#endif //RESOURCEREGISTRY_HPP
