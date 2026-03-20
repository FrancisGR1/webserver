#include <cstdlib>

#include "core/constants.hpp"
#include "http/StatusCode.hpp"
#include "ServiceConfig.hpp"


ServiceConfig::ServiceConfig()
	: redirection(StatusCode::None, "")
	, max_body_size(constants::max_body_size)
{
	Logger::trace("ServiceConfig: constructor");
}

ServiceConfig::ServiceConfig(std::vector<LocationConfig> locations_vec)
	: redirection(StatusCode::None, "")
	, max_body_size(constants::max_body_size)
{
	Logger::trace("ServiceConfig: constructor with locations' vector");
	for (size_t i = 0; i < locations_vec.size(); ++i)
	{
		locations[locations_vec[i].name] = locations_vec[i];
	}
}

void ServiceConfig::set(Directive& directive)
{
	switch(directive.type)
	{
		case Token::DirectiveServerName:
			server_name = directive.args.at(0);
			break;
		case Token::DirectiveListen:
			listeners.push_back(directive.listen);
			break;
		case Token::DirectiveErrorPage:
			{
				size_t code = atoi(directive.args.at(0).c_str());
				error_pages[code] = directive.args.at(1);
				break;
			}
		case Token::DirectiveRedirect:
			{
				size_t code = atoi(directive.args.at(0).c_str());
				redirection.code = static_cast<StatusCode::Code>(code);
				redirection.raw_path = directive.args.at(1);
				break;
			}
		case Token::DirectiveMaxBodySize:
			max_body_size = atoi(directive.args.at(0).c_str());
			break;
		default:
			throw std::runtime_error("Invalid: can't set " + token_type_to_literal(directive.type) + "\n");
	}
}

void ServiceConfig::set(LocationConfig& location)
{
	locations[location.name] = location;
}

std::ostream& operator<<(std::ostream& os, const ServiceConfig& service)
{
	os << "Server Info:\n";
	os << "Server Name: " << service.server_name << "\n";

	for (size_t i = 0; i < service.listeners.size(); i++)
	{
		os << "Listener " << i << ": " << service.listeners.at(i).host << ":" << service.listeners.at(i).port << "\n";
	}

	for (std::map<std::string, LocationConfig>::const_iterator it = service.locations.begin(); it != service.locations.end(); it++)
	{
		os << "Location: " << it->first << "\n" << it->second << "\n";
	}

	os << "Error pages:\n";
	for (std::map<size_t, std::string>::const_iterator it = service.error_pages.begin(); it != service.error_pages.end(); it++)
	{
		os << "\t" << it->first << " - " << it->second << "\n";
	}

	service.redirection.code 
	? os << "\tRedirection: " << service.redirection.code << ": " << service.redirection.raw_path << "\n"
	: os << "\tRedirection:\n";

	os << "Max body size: " << service.max_body_size << "\n";

	return os;
}
