#include <iostream>

#include <csignal> /* handle signal */
#include "core/constants.hpp"
#include "core/Logger.hpp"
#include "config/Config.hpp"
#include "server/Webserver.hpp"

void	handle_signal(int sig)
{
	(void)sig;
	Webserver::is_running = false;
	std::cout << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		Logger::error("./webserv <configurations_path>.conf");
		return 1;
	}
	try
	{
		// config
		const char* file_path = argc == 2 ? argv[1] : constants::default_conf;
		Config config(file_path);
		config.load();
		Logger::debug(config);

		// server
		Webserver server(config);
		signal(SIGINT, handle_signal);
		server.setup();
		server.run();

	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
	}

	return (0);
}
