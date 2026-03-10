#include "core/utils.hpp"
#include "core/Logger.hpp"
#include "Socket.hpp"
#include "Webserver.hpp"
#include "Connection.hpp"
#include "EventManager.hpp"

#include <iostream>

bool	Webserver::is_running = true;

Webserver::Webserver(const Config& config)
	: config_(config) {}

Webserver::~Webserver()
{
	/* fechar sockets do servidor */
	for (std::map<int, Socket*>::iterator it = server_sockets_.begin(); it != server_sockets_.end(); it++)
		delete it->second;
}

// if there's an error in setup(), server won't run
void Webserver::setup()
{
	// run all the services and create sockets from listeners
	for(size_t i = 0; i < config_.services.size(); i++)
	{
		const ServiceConfig& service = config_.services[i];
		for (size_t j = 0; j < service.listeners.size(); j++)
		{
			// make server socket
			const Listener& listener = service.listeners[j];
			Socket* socket = make_server_socket(listener, service);

			// store socket
			server_sockets_.insert(std::pair<int, Socket*>(socket->fd(), socket));
			if (events_.add(socket->fd(), EPOLLIN) == -1)
			{
				throw std::runtime_error("Failed to add socket to events at setup()");
			}

			Logger::trace("Listening on %s:%s", listener.host.c_str(), listener.port.c_str());
		}
	}
}

bool	Webserver::isServerSocket(int fd)
{
	return (server_sockets_.find(fd) != server_sockets_.end());
}

void	Webserver::run()
{
	Logger::trace("Server starts running\n");

	while (is_running)
	{
		int n_events = events_.wait();

		//@QUESTION: porquê -2?
		if (n_events == -2)
			continue ;

		//@TODO colocar try catch dentro do for loop
		//@QUESTION: porquê -1?
		for (int i = 0; i < n_events; ++i)
		{
			epoll_event& event = events_.getEvent(i);
			int event_fd = event.data.fd;

			Logger::trace("Event fd: %d\n", event_fd);
			// event error
			if (event.events & (EPOLLERR | EPOLLHUP))
			{
				Connection& conn = connection_pool_.get(event_fd);
				events_.remove(event_fd);
				connection_pool_.remove(conn);
			}
			else if (isServerSocket(event_fd))
			{
				std::cout << "this is a server socket\n";
				//@TODO verificar se é EPOLLIN
				Socket* client_socket = make_client_socket(event_fd);
				std::cout << "made client:" << client_socket->fd() << "\n";
				connection_pool_.make(*client_socket, events_);
				events_.add(client_socket->fd(), EPOLLIN | EPOLLOUT);
				std::cout << "made connection:" << client_socket->fd() << "\n";
			}
			else // is an existing connection
			{
				std::cout << "this is an existing connection\n";
				Connection& conn = connection_pool_.get(event_fd);
				conn.work(event);
				if (conn.done())
				{
					connection_pool_.remove(conn);
				}
			}
		}
	}
}

Socket* Webserver::make_server_socket(const Listener& listener, const ServiceConfig& service)
{
	struct addrinfo hints = {};
	struct addrinfo* result;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(listener.host.c_str(), listener.port.c_str(), &hints, &result) != 0)
	{
		throw std::runtime_error(utils::fmt("getaddrinfo() failed for listener %s:%s", 
					listener.host.c_str(), listener.port.c_str()));
	}
	/* criar socket, configurar, bind... */
	int socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//@TODO substituir por exception de servidor
	//@TODO recuperar antigas mensagens de erros
	if (socket_fd < 0)
	{
		throw std::runtime_error(utils::fmt("socket() failed for listener %s:%s", 
					listener.host.c_str(), listener.port.c_str()));
	}

	//@QUESTION opt 1?
	int opt = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		::freeaddrinfo(result);
		throw std::runtime_error(utils::fmt("setsockopt() failed for listener %s:%s", 
					listener.host.c_str(), listener.port.c_str()));
	}

	if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		::freeaddrinfo(result);
		throw std::runtime_error(utils::fmt("fcntl() failed for listener %s:%s", 
					listener.host.c_str(), listener.port.c_str()));
	}

	if (bind(socket_fd, result->ai_addr, result->ai_addrlen) < 0)
	{
		::freeaddrinfo(result);
		throw std::runtime_error(utils::fmt("bind() failed for listener %s:%s", 
					listener.host.c_str(), listener.port.c_str()));
	}
	::freeaddrinfo(result);

	//@QUESTION porquê 10 aqui?
	if (listen(socket_fd, 10) < 0)
	{
		throw std::runtime_error(utils::fmt("listen() failed for listener %s:%s", 
					listener.host.c_str(), listener.port.c_str()));
	}
	return new Socket(socket_fd, service);
}

//@TODO colocar throws aqui
Socket* Webserver::make_client_socket(int server_socket_fd)
{
	int fd = accept(server_socket_fd, NULL, NULL);
	if (fd < -1)
	{
		Logger::error("Failed to accept client connection!");
	}

	// @QUESTION porquê opt = 0?
	int opt = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < -1)
	{
		Logger::error("setsockopt() for client failed!");
	}

	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -2)
	{
		Logger::error("fcntl() failed to set client socket to non-blocking mode!");
	}

	std::map<int, Socket*>::iterator it = server_sockets_.find(server_socket_fd);
	if (it == server_sockets_.end())
		throw std::runtime_error("Server socket not found");

	return new Socket(fd, it->second->service());
}
