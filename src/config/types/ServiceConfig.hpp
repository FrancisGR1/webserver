#ifndef SERVICECONFIG_HPP
# define SERVICECONFIG_HPP

#include <vector>
#include <map>
#include <string>

#include "config/types/LocationConfig.hpp"

class ServiceConfig
{
	public:
		ServiceConfig();
		//exclusive to service block scope
		std::string server_name;
		std::vector<Listener> listeners;
		std::map<std::string, LocationConfig> locations;

		// can be set in LocationConfig or ServiceConfig scope
		std::map<size_t, std::string> error_pages;
		Route redirection;
		size_t max_body_size;

		void set(Directive& directive);
		void set(LocationConfig& location);

	private:
};

std::ostream& operator<<(std::ostream& os, const ServiceConfig& service);

# endif // SERVICECONFIG_HPP
