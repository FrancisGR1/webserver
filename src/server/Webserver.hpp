#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <sys/socket.h> /* socket, setsockopt */
#include <netdb.h>
#include <netinet/in.h> /* sockaddr_in */
#include <iostream>
#include <unistd.h>
#include <poll.h>
#include <string>
#include "config/Config.hpp"

/* cria o servidor ja com as config */
class Webserver
{
	private:
		std::vector<int>	server_sockets;
		std::vector<pollfd>	fds;
		int					opt_value;
		int					socket_config;
		struct sockaddr_in	addr;
		const Config		&config_;

	public:
		Webserver(const Config &config_);
		~Webserver();

		static bool is_running;
		void	startServer();
		int		setupSocket();
		void	createPoll();
		void	log(const std::string &message);
};

#endif /* WEBSERVER_HPP */