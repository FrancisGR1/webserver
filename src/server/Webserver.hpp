#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <sys/socket.h> /* socket, setsockopt */
#include <netdb.h>
#include <netinet/in.h> /* sockaddr_in */
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>
#include <string>
#include <fcntl.h>

#include "config/Config.hpp"
#include "Connection.hpp"
#include "ConnectionPool.hpp"
#include "EventManager.hpp"
#include "Socket.hpp"

class Connection;

/* cria o servidor ja com as config */
class Webserver
{
	public:
		Webserver(const Config& config);
		~Webserver();

		//@TODO implement
		void setup();
		void run();

		static bool is_running;

		void	startServer();
		int		setupSocket();
		bool	isServerSocket(int fd);
		void	acceptConnection(const int sock);
		void	handleConnection(const int sock, epoll_event& event);

		void	log(const std::string &message);

	private:
		const Config				&config_;
		std::map<int, Socket*>		server_sockets_;
		EventManager				events_;
		ConnectionPool				connection_pool_;

		// utils 
		Socket* make_server_socket(const Listener& listener, const ServiceConfig& service);
		Socket* make_client_socket(int fd);

		// illegal 
		Webserver(); // must be initialized with config

};

#endif /* WEBSERVER_HPP */
