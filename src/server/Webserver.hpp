#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <sys/socket.h> /* socket, setsockopt */
#include <netdb.h>
#include <netinet/in.h> /* sockaddr_in */
#include <iostream>
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>
#include <string>
#include <fcntl.h>
#include "config/Config.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpRequestParser.hpp"
#include "http/HttpResponse.hpp"
#include "http/StatusCode.hpp"
#include "Connection.hpp"

class Connection;

/* cria o servidor ja com as config */
class Webserver
{
	private:
		std::vector<int>			server_sockets;
		std::map<int, Connection>	connections;
		const Config				&config_;
		int							epoll_fd_;
		epoll_event					events_[1024];

	public:
		Webserver(const Config &config_);
		~Webserver();

		static bool is_running;
		void	startServer();
		int		setupSocket();
		bool	isServerSocket(int fd);
		void	addEpoll(const int socket, uint32_t events);
		void	modifyEpoll(int socket, uint32_t events);
		void	removeEpoll(int socket);
		void	acceptConnection(const int sock);
		void	handleConnection(const int sock, epoll_event event);

		void	log(const std::string &message);
};

#endif /* WEBSERVER_HPP */