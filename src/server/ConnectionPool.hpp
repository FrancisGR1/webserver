#ifndef CONNECTIONPOOL_HPP
#define CONNECTIONPOOL_HPP

#include <list>

#include "Connection.hpp"

class ConnectionPool
{
	public:
		ConnectionManager& make(const Socket& conn_socket, EventManager& events);
		void add_key_to_existing_connection(int key, ConnectionManager& conn);

		//@TODO: ainda não sei a melhor forma de remover?
		void remove(ConnectionManager& conn);

		ConnectionManager& get(int fd);

	private:
		// connections can have multiple associated ints
		std::list<ConnectionManager> connection_pool_; // std::list so connection pointers are stable
		std::map<int, ConnectionManager*>	conn_by_fd_;
};

#endif // CONNECTIONPOOL_HPP
