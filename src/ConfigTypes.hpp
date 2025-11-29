#ifndef CONFIGTYPES_HPP
# define CONFIGTYPES_HPP

#include <string>
#include <ostream>
#include <cstddef>
#include <vector>
#include <set>
#include <map>

// ==============================================================
// Keywords namespace
// ==============================================================

namespace ConfigKeywords
{
	static const std::set<std::string> delimiters  = 
	{ 
		"{", 
		"}", 
		";" 
	};

	static const std::set<std::string> block = 
	{ 
		"service", 
		"location"
	};

	static const std::set<std::string> directive = 
	{ 
		"listen", 
		"server_name", 
		"max_body_size", 
		"error_page",
		"root", "methods", 
		"default_file",
		"listing",
		"upload", 
		"upload_dir",
		"cgi",
		"redirect" 
	};

	static const std::set<std::string> cgi_extensions =
	{
		".py"
		// add extensions here
	};
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

	Token(Token::Type type, std::string value, size_t start_pos)
		: type(type)
		, value(value)
		, start_pos(start_pos)
	{}

	Token(std::string value, size_t start_pos)
		: value(value)
		, start_pos(start_pos)
		, type(classify())
	{}

	//classifies its type
	Token::Type classify();
	void classify_word_in_context(Token::Type context);
	void classify_block();
	void classify_directive();

	std::string to_string() const;
	std::string to_literal() const;


	std::string value;
	size_t start_pos;
	Token::Type type;
};

std::ostream& operator<<(std::ostream& os, const Token& t);
std::string token_type_to_literal(Token::Type type);

struct Listener
{
	std::string host;
	size_t port;
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
	// exclusive to location block scope
	std::string name;
	std::set<std::string> methods;
	std::string root_dir;
	std::string upload_dir;
	bool enable_dir_listing = false;
	bool enable_upload_files = false;
	std::string default_file;
	std::map<std::string, std::string> cgis;

	// can be set in location or service scope
	std::map<size_t, std::string> error_pages;
	std::map<size_t, std::string> redirections;
	size_t max_body_size = 1000000;

	void set(Directive& directive);
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& lc);

struct ServiceConfig
{
	//exclusive to service block scope
	std::string server_name;
	std::vector<Listener> listeners;
	std::map<std::string, LocationConfig> locations;

	// can be set in location or service scope
	std::map<size_t, std::string> error_pages;
	std::map<size_t, std::string> redirections;
	size_t max_body_size = 1000000;

	void set(Directive& directive);
	void set(LocationConfig& location);
};

std::ostream& operator<<(std::ostream& os, const ServiceConfig& service);

#endif //CONFIGTYPES_HPP
