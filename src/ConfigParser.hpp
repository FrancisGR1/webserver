#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include <string>
#include <ostream>
#include "ConfigTypes.hpp"
#include "Config.hpp"

class ConfigLexer;
class Config;

class ConfigParser
{
	public:
		ConfigParser(ConfigLexer& lexer);
		friend std::ostream& operator<<(std::ostream& os, const ConfigParser& parser);
		const Config& get();

	private:
		ConfigLexer& m_lexer;
		Config m_config;

		bool is_directive(const std::string& str);
		bool is_block(const std::string& str);
		bool is_directive_name(const std::string& str);

		void parse();
		void parse_service(Token& t, ServiceConfig& service);
		void parse_location(Token& t, LocationConfig& location);
		const std::string& parse_location_dir(const Token& t);
		void parse_directive(Token& t, Token::Type context, Directive& directive);

		void parse_listener(Token& t, Directive& directive);
		void parse_server_name(Token& t, Directive& directive);
		void parse_upload_dir(Token& t, Directive& directive);
		void parse_max_body_size(Token& t, Directive& directive);
		void parse_error_page(Token& t, Directive& directive);
		void parse_root(Token& t, Directive& directive);
		void parse_methods(Token& t, Directive& directive);
		void parse_default_file(Token& t, Directive& directive);
		void parse_listing(Token& t, Directive& directive);
		void parse_upload(Token& t, Directive& directive);
		void parse_cgi(Token& t, Directive& directive);
		void parse_redirect(Token& t, Directive& directive);

		void expect(const Token& t, Token::Type expected);
		void expect(const Token& t, char expected, size_t pos);
		void expect(const Token& t, Token::Type expected_type, char expected_ch, size_t pos);
};

#endif //CONFIGPARSER_HPP
