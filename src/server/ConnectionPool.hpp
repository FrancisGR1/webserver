#ifndef CONNECTIONPOOL_HPP
#define CONNECTIONPOOL_HPP

#include <list>

#include "Connection.hpp"

class ConnectionPool
{
	public:
		ConnectionPool(EventManager& events);
		Connection& make(Socket& conn_socket);
		void associate_fd(int fd, Connection& conn);

		//@TODO: ainda não sei a melhor forma de remover?
		void remove(Connection& conn);
		void remove(int fd); //@TODO: impl

		Connection& get(int fd);

	private:
		EventManager& events_;
		// connections can have multiple associated ints
		std::list<Connection*> connection_pool_; // std::list so connection pointers are stable
		std::map<int, Connection*>	conn_by_fd_;
};

#endif // CONNECTIONPOOL_HPP
