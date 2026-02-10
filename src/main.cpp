#include <iostream>

#include <csignal> /* handle signal */
#include "core/constants.hpp"
#include "core/Logger.hpp"
#include "config/Config.hpp"
#include "server/Webserver.hpp"
#include "server/Client.hpp"

void	handle_signal(int sig)
{
	(void)sig;
	Webserver::is_running = false;

}

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		Logger::error("./webserv <configurations_path>.conf");
		return 1;
	}
	const char* file_path = argc == 2 ? argv[1] : constants::default_conf;
	Config config(file_path);
	config.load();
	Logger::trace(config);

	Webserver server(config);

	signal(SIGINT, handle_signal);

	server.startServer();

	return (0);
}
