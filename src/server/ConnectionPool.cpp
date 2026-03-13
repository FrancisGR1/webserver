#include <iostream>

#include "core/utils.hpp"
#include "core/Logger.hpp"
#include "ConnectionPool.hpp"


// make connection, store it, return a reference to it
Connection& ConnectionPool::make(Socket& conn_socket, EventManager& events)
{
	if (utils::contains(conn_by_fd_, conn_socket.fd()))
		throw std::logic_error("Connection Pool: already contains this fd!\n");

	// create and save connection with its socket fd
	connection_pool_.insert(connection_pool_.end(), new Connection(conn_socket, events));
	conn_by_fd_[conn_socket.fd()] = connection_pool_.back();
	return *conn_by_fd_[conn_socket.fd()];
}

void ConnectionPool::associate_fd(int fd, Connection& conn)
{
	if (utils::contains(conn_by_fd_, fd))
		throw std::logic_error("Connection Pool: already contains this fd!\n");

	for (std::list<Connection*>::iterator it = connection_pool_.begin();
			it != connection_pool_.end(); ++it)
	{
		if (*it == &conn)
		{
			throw std::logic_error("Connection Pool: connection not owned by pool!");
		}
	}

	conn_by_fd_[fd] = &conn;
}

void ConnectionPool::remove(Connection& conn)
{
	// map
	for (std::map<int, Connection*>::iterator it = conn_by_fd_.begin(); it != conn_by_fd_.end(); )
	{
		if (it->second == &conn)
		{
			std::map<int, Connection*>::iterator tmp = it;
			++it;
			conn_by_fd_.erase(tmp);
		}
		else
		{
			++it;
		}
	}

	// list
	for (std::list<Connection*>::iterator it = connection_pool_.begin();
			it != connection_pool_.end(); )
	{
		if (*it == &conn)
		{
			connection_pool_.erase(it);
			break;
		}
	}
}


Connection& ConnectionPool::get(int fd)
{
	Logger::trace("Connection Pool: Get fd %d", fd);

	if (!utils::contains(conn_by_fd_, fd))
		throw std::logic_error("Connection Pool: looking up fd that doesn't exist!\n");

	return *conn_by_fd_[fd];
}
