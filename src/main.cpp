#include <iostream>

#include "core/constants.hpp"
#include "core/Logger.hpp"
#include "config/Config.hpp"

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
}
