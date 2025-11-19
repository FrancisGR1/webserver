#pragma once
#include "webserver.hpp"

//https://github.com/h5bp/server-configs-nginx
//
struct Listeners
{
	std::string host;
	uint16_t port;
};

struct RouteInfo
{
	std::set<std::string> methods;
	std::string root_dir;
	bool dir_listing;
	bool upload_file;
	std::string default_file;
	std::map<std::string, std::string> cgis;
};

struct ServiceConfig
{
	std::vector<Listeners> listeners;
	std::map<std::string, RouteInfo> routes;
	size_t max_body_size;
	std::map<int, std::string> error_pages;
};

class Configurations 
{
	public: 
		Configurations(const std::string& file_path);
		std::vector<ServiceConfig> services;
		//<<

	private:

		std::ifstream m_file_content;

		// directives
		enum class Directive
		{
			Method,
			Root,
			CGI, 
			Upload,
			Upload_Dir,
			Listing,
			Listen,
			Server_Name,
			Unkown,
		};

		std::map<std::string, Directive> m_directives;

		void initDirectives() const;
		Directive lookupDirective(const std::string& directive) const;


		// parsing
		void parse();
		//... func para cada diretiva
};

//@TODO friend?
friend std::ostream& operator<<(std::ostream& os, const Configurations& cfg);
