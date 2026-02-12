#ifndef CONFIGTYPES_HPP
# define CONFIGTYPES_HPP

#include <string>
#include <ostream>
#include <cstddef>
#include <vector>
#include <set>
#include <map>
#include <utility>

#include "http/StatusCode.hpp"

// ==============================================================
// Keywords namespace
// ==============================================================

namespace ConfigKeywords
{
    const std::set<std::string> make_delimiters();
    const std::set<std::string> make_block();
    const std::set<std::string> make_directive();
    const std::set<std::string> make_cgi_extensions();

    const std::set<std::string> delimiters = make_delimiters();
    const std::set<std::string> block = make_block();
    const std::set<std::string> directive = make_directive();
    const std::set<std::string> cgi_extensions = make_cgi_extensions();
}

// ==============================================================
// Token struct
// ==============================================================

struct Token
{
	enum Type
	{
		// lexer types
		Word,
		Semicolon,
		LBrace,
		RBrace,
		Eof,
		Unknown,

		// block types
		NoBlock,
		Service,
		Location,

		// directive statement parts
		Directive,
		DirectiveName,
		DirectiveParameter,

		// specify the directive type further
		DirectiveListen, 
		DirectiveServerName,
		DirectiveMaxBodySize,
		DirectiveErrorPage,
		DirectiveRoot,
		DirectiveMethods,
		DirectiveDefaultFile,
		DirectiveListing,
		DirectiveUpload,
		DirectiveUploadDir,
		DirectiveCgi,
		DirectiveRedirect
	};

	Token(Token::Type type, std::string value, size_t start_pos);
	Token(std::string value, size_t start_pos);

	//classifies its type
	Token::Type classify();
	void classify_word_in_context(Token::Type context);
	void classify_block();
	void classify_directive();

	std::string to_string() const;
	std::string to_literal() const;


	std::string value;
	Token::Type type;
	size_t start_pos;
};

std::ostream& operator<<(std::ostream& os, const Token& t);
std::string token_type_to_literal(Token::Type type);

// ==============================================================
// Config Types
// ==============================================================
struct Listener
{
	std::string host;
	std::string port;
};

struct Route
{
	Route(StatusCode::Code code, std::string path);

	StatusCode::Code code;
	std::string path;
};

struct Directive
{
	std::string name;
	Token::Type type;
	size_t start_pos;

	// a directive has either one of these
	Listener listen;
	std::vector<std::string> args;
};

struct LocationConfig
{
	LocationConfig();
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
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& lc);

struct ServiceConfig
{
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
};

std::ostream& operator<<(std::ostream& os, const ServiceConfig& service);

#endif //CONFIGTYPES_HPP
