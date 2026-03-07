#include "core/utils.hpp"
#include "ConnectionPool.hpp"


// make connection, store it, return a reference
ConnectionManager& ConnectionPool::make(const Socket& conn_socket, EventManager& events)
{
	if (utils::contains(conn_by_fd_, conn_socket.getFd()))
		throw std::logic_error("Connection Pool: already contains this fd!\n");

	// create and save connection with its socket fd
	connection_pool_.push_back(ConnectionManager(conn_socket, events));
	conn_by_fd_[conn_socket.getFd()] = &connection_pool_.back();
	return *conn_by_fd_[conn_socket.getFd()];
}

void ConnectionPool::add_key_to_existing_connection(int key, ConnectionManager& conn)
{
	if (utils::contains(conn_by_fd_, key))
		throw std::logic_error("Connection Pool: already contains this fd!\n");

	for (std::list<ConnectionManager>::iterator it = connection_pool_.begin();
			it != connection_pool_.end(); ++it)
	{
		if (&(*it) == &conn)
		{
			throw std::logic_error("Connection Pool: connection not owned by pool!");
		}
	}

	conn_by_fd_[key] = &conn;
}

void ConnectionPool::remove(ConnectionManager& conn)
{
	// list
	for (std::list<ConnectionManager>::iterator it = connection_pool_.begin();
			it != connection_pool_.end(); )
	{
		if (&(*it) == &conn)
		{
			connection_pool_.erase(it);
			break;
		}
	}

	// map
	for (std::map<int, ConnectionManager>::iterator it = conn_by_fd_.begin(); it != conn_by_fd_.end(); )
	{
		if (it->second == &conn)
		{
			it = conn_by_fd_.erase(it);
		}
		else
		{
			++it;
		}
	}
}

ConnectionManager& ConnectionPool::get(int fd)
{
	if (!utils::contains(conn_by_fd_, fd))
		throw std::logic_error("Connection Pool: looking up fd that doesn't exist!\n");

	return *conn_by_fd_[fd];
}
