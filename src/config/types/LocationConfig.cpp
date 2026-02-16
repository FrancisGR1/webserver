#include <cstdlib>
#include <sstream>
#include <set>
#include <string>
#include <map>

#include "core/constants.hpp"
#include "http/StatusCode.hpp"
#include "Directive.hpp"
#include "LocationConfig.hpp"

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

