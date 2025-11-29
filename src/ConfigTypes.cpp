#include "ConfigTypes.hpp"
#include <iomanip>

// ==============================================================
// Token struct
// ==============================================================

std::string Token::to_string() const
{
	switch(type) 
	{
		case Token::Type::Word:      return "Word";
		case Token::Type::Semicolon: return "Semicolon";
		case Token::Type::LBrace:    return "LBrace";
		case Token::Type::RBrace:    return "RBrace";
		case Token::Type::Eof:       return "Eof";

		case Token::Type::Service:   return "Service";
		case Token::Type::Location:  return "Location";
		case Token::Type::Directive: return "Directive";
		case Token::Type::DirectiveName: return "DirectiveName";
		case Token::Type::DirectiveParameter: return "DirectiveParameter";

		default:                     return "Unknown";
	}
}

std::string Token::to_literal() const
{
	switch(type) 
	{
		case Token::Type::Word:      return value;
		case Token::Type::Semicolon: return ";";
		case Token::Type::LBrace:    return "{";
		case Token::Type::RBrace:    return "}";
		case Token::Type::Eof:       return "Eof";

		case Token::Type::Service:   return "Service";
		case Token::Type::Location:  return "Location";
		case Token::Type::Directive: return "Directive";
		case Token::Type::DirectiveName: return value;
		case Token::Type::DirectiveParameter: return value;

		default:                     return "Unknown";
	}
}

std::string token_type_to_literal(Token::Type type)
{
	switch(type)
	{
		case Token::Type::Word: return "word";
		case Token::Type::Semicolon: return ";";
		case Token::Type::LBrace: return "{";
		case Token::Type::RBrace: return "}";
		case Token::Type::Eof: return "Eof";
		case Token::Type::Unknown: return "Unknown";

		// block types
		case Token::Type::NoBlock: return "NoBlock";
		case Token::Type::Service: return "service";
		case Token::Type::Location: return "location";

		// directive statement parts
		case Token::Type::Directive: return "Directive";
		case Token::Type::DirectiveName: return "DirectiveName";
		case Token::Type::DirectiveParameter: return "DirectiveParameter";

		// specify the directive type further
		case Token::Type::DirectiveListen:  return "DirectiveListen";
		case Token::Type::DirectiveServerName: return "DirectiveServerName";
		case Token::Type::DirectiveMaxBodySize: return "DirectiveMaxBodySize";
		case Token::Type::DirectiveErrorPage: return "DirectiveErrorPage";
		case Token::Type::DirectiveRoot: return "DirectiveRoot";
		case Token::Type::DirectiveMethods: return "DirectiveMethods";
		case Token::Type::DirectiveDefaultFile: return "DirectiveDefaultFile";
		case Token::Type::DirectiveListing: return "DirectiveListing";
		case Token::Type::DirectiveUpload: return "DirectiveUpload";
		case Token::Type::DirectiveUploadDir: return "DirectiveUploadDir";
		case Token::Type::DirectiveCgi: return "DirectiveCgi";
		case Token::Type::DirectiveRedirect: return "DirectiveRedirect";
	}
	return "Unknown";
}

std::ostream& operator<<(std::ostream& os, const Token& t)
{
	os << std::left               
	   << std::setw(12)   
	   << t.to_string()
	   << " -> "
	   << t.value;
	return os;
};

Token::Type Token::classify()
{
	if      (value == "{")    return Token::Type::LBrace;
	else if (value == "}")    return Token::Type::RBrace;
	else if (value == ";")    return Token::Type::Semicolon;
	else                      return Token::Type::Word; // if its not a delimiter, it defaults to a word
}

void Token::classify_word_in_context(Token::Type context)
{
	switch (context)
	{
		case Token::Type::NoBlock: 
			type = Token::Type::Service; 
			break;
		case Token::Type::Service:
			if (value == "location") type = Token::Type::Location;
			else                     type = Token::Type::DirectiveName;
			break;
		case Token::Type::Location: 
			type = Token::Type::DirectiveName; 
			break;
		case Token::Type::DirectiveName:
			classify_directive();
			break;
	}
}

void Token::classify_block()
{
	if      (value == "service")  type = Token::Type::Service;
	else if (value == "location") type = Token::Type::Location;
	else                          type = Token::Type::Unknown;
}

void Token::classify_directive()
{
	if      (value == "listen")        type = Token::Type::DirectiveListen;
	else if (value == "server_name")   type = Token::Type::DirectiveServerName;
	else if (value == "max_body_size") type = Token::Type::DirectiveMaxBodySize;
	else if (value == "error_page")    type = Token::Type::DirectiveErrorPage;
	else if (value == "root")          type = Token::Type::DirectiveRoot;
	else if (value == "methods")       type = Token::Type::DirectiveMethods;
	else if (value == "default_file")  type = Token::Type::DirectiveDefaultFile;
	else if (value == "listing")       type = Token::Type::DirectiveListing;
	else if (value == "upload")        type = Token::Type::DirectiveUpload;
	else if (value == "upload_dir")    type = Token::Type::DirectiveUploadDir;
	else if (value == "cgi")           type = Token::Type::DirectiveCgi;
	else if (value == "return")      type = Token::Type::DirectiveRedirect;
	else                               type = Token::Type::DirectiveParameter;
}


// ==============================================================
// LocationConfig struct
// ==============================================================

void LocationConfig::set(Directive& directive)
{
	switch(directive.type)
	{
		case Token::Type::DirectiveMethods:
			{
				methods.insert(directive.args.begin(), directive.args.end());
				break;
			}
		case Token::Type::DirectiveRoot:
			{
				root_dir = directive.args.at(0);
				break;
			}
		case Token::Type::DirectiveListing:
			{
				enable_dir_listing = directive.args.at(0) == "on";
				break;
			}
		case Token::Type::DirectiveUpload:
			{
				enable_upload_files = directive.args.at(0) == "on";
				break;
			}
		case Token::Type::DirectiveUploadDir:
			{
				upload_dir = directive.args.at(0);
				break;
			}
		case Token::Type::DirectiveDefaultFile:
			{
				default_file = directive.args.at(0);
				break;
			}
		case Token::Type::DirectiveCgi:
			{
				std::string key = directive.args.at(0);
				cgis[key] = directive.args.at(1);
				break;
			}
		case Token::Type::DirectiveErrorPage:
			{
				size_t code = std::atoi(directive.args.at(0).c_str());
				error_pages[code] = directive.args.at(1);
				break;
			}
		case Token::Type::DirectiveRedirect:
			{
				size_t code = std::atoi(directive.args.at(0).c_str());
				redirections[code] = directive.args.at(1);
				break;
			}
		case Token::Type::DirectiveMaxBodySize:
			{
				max_body_size = std::atoi(directive.args.at(0).c_str());
				break;
			}
		default:
			throw std::runtime_error("Invalid: can't set " + token_type_to_literal(directive.type) + "\n");
	}
}

std::ostream& operator<<(std::ostream& os, const LocationConfig& lc)
{
	os << "\tLocation Name: " << lc.name << "\n";
	os << "\tMethods: ";
	for (std::set<std::string>::const_iterator it = lc.methods.begin(); it != lc.methods.end(); it++) 
	{
		os << *it;
		if (it != std::prev(lc.methods.end()))
		{
			os << ", ";
		}
	}
	os << "\n";
	os << "\tRoot Dir: " << lc.root_dir << "\n";
	os << "\tDir Listing: " << (lc.enable_dir_listing ? "on" : "off") << "\n";
	os << "\tUpload Files: " << (lc.enable_upload_files ? "on" : "off") << "\n";
	os << "\tUpload Dir: " << lc.upload_dir << "\n";
	os << "\tDefault File: " << lc.default_file << "\n";

	os << "\tCgi:\n";
	for (std::map<std::string, std::string>::const_iterator it = lc.cgis.begin(); it != lc.cgis.end(); it++)
	{
		os << "\t\t" << it->first << " " << it->second << "\n";
	}

	os << "\tError pages:\n";
	for (std::map<size_t, std::string>::const_iterator it = lc.error_pages.begin(); it != lc.error_pages.end(); it++)
	{
		os << "\t\t" << it->first << " - " << it->second << "\n";
	}

	os << "\tRedirections:\n";
	for (std::map<size_t, std::string>::const_iterator it = lc.redirections.begin(); it != lc.redirections.end(); it++)
	{
		os << "\t\t" << it->first << " - " << it->second << "\n";
	}
	os << "\tMax Body Size: " << lc.max_body_size << "\n";

	return os;
}

// ==============================================================
// ServiceConfig struct
// ==============================================================

void ServiceConfig::set(Directive& directive)
{
	switch(directive.type)
	{
		case Token::Type::DirectiveServerName:
			server_name = directive.args.at(0);
			break;
		case Token::Type::DirectiveListen:
			listeners.push_back(directive.listen);
			break;
		case Token::Type::DirectiveErrorPage:
			{
				size_t code = std::atoi(directive.args.at(0).c_str());
				error_pages[code] = directive.args.at(1);
				break;
			}
		case Token::Type::DirectiveRedirect:
			{
				size_t code = std::atoi(directive.args.at(0).c_str());
				redirections[code] = directive.args.at(1);
				break;
			}
		case Token::Type::DirectiveMaxBodySize:
			max_body_size = std::atoi(directive.args.at(0).c_str());
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

	os << "Redirections:\n";
	for (std::map<size_t, std::string>::const_iterator it = service.redirections.begin(); it != service.redirections.end(); it++)
	{
		os << "\t" << it->first << " - " << it->second << "\n";
	}

	os << "Max body size: " << service.max_body_size << "\n";

	return os;
}
