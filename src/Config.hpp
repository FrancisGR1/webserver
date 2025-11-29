#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <string>
#include <ostream>
#include "ConfigTypes.hpp"

//https://github.com/h5bp/server-configs-nginx

class Config 
{
	public: 
		Config();
		Config(const char *file_path);

		std::vector<ServiceConfig> services;

		void load();


	private:
		std::string m_file_content;
};

std::ostream& operator<<(std::ostream& os, const Config& cfg);

#endif //CONFIG_HPP
