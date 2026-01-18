#include "core/constants.hpp"
#include "ConfigTypes.hpp"
#include <cstdlib>
#include <iomanip>

// ==============================================================
// Keywords namespace
// ==============================================================

namespace ConfigKeywords
{
    const std::set<std::string> make_delimiters()
    {
        std::set<std::string> s;
        s.insert("{");
        s.insert("}");
        s.insert(";");
        return s;
    }

    const std::set<std::string> make_block()
    {
        std::set<std::string> s;
        s.insert("service");
        s.insert("location");
        return s;
    }

    const std::set<std::string> make_directive()
    {
        std::set<std::string> s;
        s.insert("listen");
        s.insert("server_name");
        s.insert("max_body_size");
        s.insert("error_page");
        s.insert("root");
        s.insert("methods");
        s.insert("default_file");
        s.insert("listing");
        s.insert("upload");
        s.insert("upload_dir");
        s.insert("cgi");
        s.insert("redirect");
        return s;
    }

    const std::set<std::string> make_cgi_extensions()
    {
        std::set<std::string> s;
        s.insert(".py");
        // Add extensions here
        return s;
    }
}

// ==============================================================
// Token struct
// ==============================================================

Token::Token(Token::Type type, std::string value, size_t start_pos)
		: value(value)
		, type(type)
		, start_pos(start_pos)
{}

Token::Token(std::string value, size_t start_pos)
		: value(value)
		, type(classify())
		, start_pos(start_pos)
{}

std::string Token::to_string() const
{
	switch(type) 
	{
		case Token::Word:      return "Word";
		case Token::Semicolon: return "Semicolon";
		case Token::LBrace:    return "LBrace";
		case Token::RBrace:    return "RBrace";
		case Token::Eof:       return "Eof";

		case Token::Service:   return "Service";
		case Token::Location:  return "Location";
		case Token::Directive: return "Directive";
		case Token::DirectiveName: return "DirectiveName";
		case Token::DirectiveParameter: return "DirectiveParameter";

		default:                     return "Unknown";
	}
}

std::string Token::to_literal() const
{
	switch(type) 
	{
		case Token::Word:      return value;
		case Token::Semicolon: return ";";
		case Token::LBrace:    return "{";
		case Token::RBrace:    return "}";
		case Token::Eof:       return "Eof";

		case Token::Service:   return "Service";
		case Token::Location:  return "Location";
		case Token::Directive: return "Directive";
		case Token::DirectiveName: return value;
		case Token::DirectiveParameter: return value;

		default:                     return "Unknown";
	}
}

std::string token_type_to_literal(Token::Type type)
{
	switch(type)
	{
		case Token::Word: return "word";
		case Token::Semicolon: return ";";
		case Token::LBrace: return "{";
		case Token::RBrace: return "}";
		case Token::Eof: return "Eof";
		case Token::Unknown: return "Unknown";

		// block types
		case Token::NoBlock: return "NoBlock";
		case Token::Service: return "service";
		case Token::Location: return "location";

		// directive statement parts
		case Token::Directive: return "Directive";
		case Token::DirectiveName: return "DirectiveName";
		case Token::DirectiveParameter: return "DirectiveParameter";

		// specify the directive type further
		case Token::DirectiveListen:  return "DirectiveListen";
		case Token::DirectiveServerName: return "DirectiveServerName";
		case Token::DirectiveMaxBodySize: return "DirectiveMaxBodySize";
		case Token::DirectiveErrorPage: return "DirectiveErrorPage";
		case Token::DirectiveRoot: return "DirectiveRoot";
		case Token::DirectiveMethods: return "DirectiveMethods";
		case Token::DirectiveDefaultFile: return "DirectiveDefaultFile";
		case Token::DirectiveListing: return "DirectiveListing";
		case Token::DirectiveUpload: return "DirectiveUpload";
		case Token::DirectiveUploadDir: return "DirectiveUploadDir";
		case Token::DirectiveCgi: return "DirectiveCgi";
		case Token::DirectiveRedirect: return "DirectiveRedirect";
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
	if      (value == "{")    return Token::LBrace;
	else if (value == "}")    return Token::RBrace;
	else if (value == ";")    return Token::Semicolon;
	else                      return Token::Word; // if its not a delimiter, it defaults to a word
}

void Token::classify_word_in_context(Token::Type context)
{
	switch (context)
	{
		case Token::NoBlock: 
			type = Token::Service; 
			break;
		case Token::Service:
			if (value == "location") type = Token::Location;
			else                     type = Token::DirectiveName;
			break;
		case Token::Location: 
			type = Token::DirectiveName; 
			break;
		case Token::DirectiveName:
			classify_directive();
			break;
		default:
			type = Token::Unknown;
	}
}

void Token::classify_block()
{
	if      (value == "service")  type = Token::Service;
	else if (value == "location") type = Token::Location;
	else                          type = Token::Unknown;
}

void Token::classify_directive()
{
	if      (value == "listen")        type = Token::DirectiveListen;
	else if (value == "server_name")   type = Token::DirectiveServerName;
	else if (value == "max_body_size") type = Token::DirectiveMaxBodySize;
	else if (value == "error_page")    type = Token::DirectiveErrorPage;
	else if (value == "root")          type = Token::DirectiveRoot;
	else if (value == "methods")       type = Token::DirectiveMethods;
	else if (value == "default_file")  type = Token::DirectiveDefaultFile;
	else if (value == "listing")       type = Token::DirectiveListing;
	else if (value == "upload")        type = Token::DirectiveUpload;
	else if (value == "upload_dir")    type = Token::DirectiveUploadDir;
	else if (value == "cgi")           type = Token::DirectiveCgi;
	else if (value == "return")        type = Token::DirectiveRedirect;
	else                               type = Token::DirectiveParameter;
}


Route::Route(StatusCode::Code code, std::string path)
	: code(code)
	, path(path) {}

// ==============================================================
// LocationConfig struct
// ==============================================================

LocationConfig::LocationConfig()
	: name("")
	, enable_dir_listing(false)
	, enable_upload_files(false)
	, redirection(StatusCode::None, "")
	, max_body_size(constants::max_body_size)
{}

void LocationConfig::set(Directive& directive)
{
	switch(directive.type)
	{
		case Token::DirectiveMethods:
			{
				methods.insert(directive.args.begin(), directive.args.end());
				break;
			}
		case Token::DirectiveRoot:
			{
				root_dir = directive.args.at(0);
				break;
			}
		case Token::DirectiveListing:
			{
				enable_dir_listing = directive.args.at(0) == "on";
				break;
			}
		case Token::DirectiveUpload:
			{
				enable_upload_files = directive.args.at(0) == "on";
				break;
			}
		case Token::DirectiveUploadDir:
			{
				upload_dir = directive.args.at(0);
				break;
			}
		case Token::DirectiveDefaultFile:
			{
				default_file = directive.args.at(0);
				break;
			}
		case Token::DirectiveCgi:
			{
				std::string key = directive.args.at(0);
				cgis[key] = directive.args.at(1);
				break;
			}
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
				redirection.path = directive.args.at(1);
				break;
			}
		case Token::DirectiveMaxBodySize:
			{
				max_body_size = atoi(directive.args.at(0).c_str());
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
		if (it != --lc.methods.end())
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

	lc.redirection.code 
	? os << "\tRedirection: " << lc.redirection.code << ": " << lc.redirection.path << "\n"
	: os << "\tRedirection:\n";
	os << "\tMax Body Size: " << lc.max_body_size << "\n";

	return os;
}

// ==============================================================
// ServiceConfig struct
// ==============================================================

ServiceConfig::ServiceConfig()
	: redirection(StatusCode::None, "")
	, max_body_size(constants::max_body_size)
{}

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
				redirection.path = directive.args.at(1);
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
	? os << "\tRedirection: " << service.redirection.code << ": " << service.redirection.path << "\n"
	: os << "\tRedirection:\n";

	os << "Max body size: " << service.max_body_size << "\n";

	return os;
}
