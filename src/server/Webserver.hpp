#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <sys/socket.h> /* socket, setsockopt */
#include <netinet/in.h> /* sockaddr_in */
#include <iostream>
#include <unistd.h>

class Webserver
{
	private:
		int					server_fd;
		int					opt_value;
		int					server_config;
		struct sockaddr_in	addr;

	public:
		Webserver();
		~Webserver();

		static bool is_running;
		void	startServer();
		int		setupSocket();
		void	log(const std::string &message);
};

#endif /* WEBSERVER_HPP */