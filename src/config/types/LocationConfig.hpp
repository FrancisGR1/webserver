#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

#include <string>
#include <map>
#include <set>

#include "config/types/Route.hpp"
#include "config/types/Directive.hpp"
#include "config/ConfigTypes.hpp"

//@TODO mudar variáveis para vars privadas
class LocationConfig
{
	public:
		LocationConfig();
		LocationConfig(const std::string name, const std::string root_dir);

		// exclusive to location block scope
		std::string name;
		std::set<std::string> methods;
		//@TODO: mudar para Path
		std::string root_dir;
		std::string upload_dir;
		bool enable_dir_listing;
		bool enable_upload_files;
		std::string default_file;
		std::map<std::string, std::string> cgis;

		// can be set in LocationConfig or ServiceConfig scope
		std::map<size_t, std::string> error_pages;
		Route redirection;
		size_t max_body_size;

		void set(Directive& directive);

	private:
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& lc);

#endif // LOCATIONCONFIG_HPP
