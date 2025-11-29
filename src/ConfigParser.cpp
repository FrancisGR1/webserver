#include <string>
#include <ostream>
#include "utils.hpp"
#include "ConfigTypes.hpp"
#include "ConfigLexer.hpp"
#include "ConfigParser.hpp"
#include "Config.hpp"

ConfigParser::ConfigParser(ConfigLexer& lexer)
	: m_lexer(lexer)
	, m_config()
{
	parse();
}

const Config& ConfigParser::get()
{
	return (m_config);
}

std::ostream& operator<<(std::ostream& os, const ConfigParser& parser)
{
	os << parser.m_config << "\n";
	return os;
}

void ConfigParser::parse()
{
	Token t = m_lexer.advance();
	while (t.type != Token::Type::Eof)
	{
		t.classify_word_in_context(Token::Type::NoBlock);
		switch (t.type)
		{
			case Token::Type::Service:
				{
					ServiceConfig service;
					parse_service(t, service);
					m_config.services.push_back(service);
					break ;
				}
			default: throw std::runtime_error("Error: Invalid1\n");
		}
	}
}

void ConfigParser::parse_service(Token& t, ServiceConfig& service)
{
	expect(t, Token::Type::Service);
	t = m_lexer.advance();
	expect(t, Token::Type::LBrace);
	t = m_lexer.advance();

	while (t.type != Token::Type::RBrace && t.type != Token::Type::Eof)
	{
		t.classify_word_in_context(Token::Type::Service);

		switch(t.type)
		{
			case Token::Type::Location:
				{
					LocationConfig location;
					parse_location(t, location);
					service.set(location);
					break;
				}
			case Token::Type::DirectiveName:
				{
					Directive directive;
					parse_directive(t, Token::Type::Service, directive);
					service.set(directive);
					break;
				}
			default:
				{
					throw std::runtime_error("Error: Invalid2\n");
				}
		}
	}

	expect(t, Token::Type::RBrace);
	t = m_lexer.advance();
}

const std::string& ConfigParser::parse_location_dir(const Token& t)
{
	const std::string& location = t.value;
	expect(t, '/', 0);
	return location;
}

void ConfigParser::parse_location(Token& t, LocationConfig& location)
{
	expect(t, Token::Type::Location);
	const Token token_dir = m_lexer.advance();
	location.name = parse_location_dir(token_dir);
	t = m_lexer.advance();
	expect(t, Token::Type::LBrace);
	t = m_lexer.advance();

	while (t.type != Token::Type::RBrace && t.type != Token::Type::Eof)
	{
		t.classify_word_in_context(Token::Type::Location);
		Directive directive;
		parse_directive(t, Token::Type::Location, directive);
		location.set(directive);
	}

	expect(t, Token::Type::RBrace);
	t = m_lexer.advance();
}

void ConfigParser::parse_directive(Token& t, Token::Type context, Directive& directive)
{
	expect(t, Token::Type::DirectiveName);
	directive.name = t.value;

	t.classify_word_in_context(Token::Type::DirectiveName);
	directive.type = t.type;

	//@TODO: colocar aqui um check de contexto em vez dos throws abaixo
	switch(t.type)
	{
		case Token::Type::DirectiveListen: 
			if (context != Token::Type::Service)
				throw std::runtime_error("Invalid3\n");
			parse_listener(t, directive);
			break;
		case Token::Type::DirectiveServerName: 
			if (context != Token::Type::Service)
				throw std::runtime_error("Invalid4\n");
			parse_server_name(t, directive);
			break;

			case Token::Type::DirectiveUploadDir: parse_upload_dir(t, directive);       break;
			case Token::Type::DirectiveMaxBodySize: parse_max_body_size(t, directive);  break;
			case Token::Type::DirectiveErrorPage: parse_error_page(t, directive);       break;
			case Token::Type::DirectiveRoot: parse_root(t, directive);                  break;
			case Token::Type::DirectiveMethods: parse_methods(t, directive);            break;
			case Token::Type::DirectiveDefaultFile: parse_default_file(t, directive);   break;
			case Token::Type::DirectiveListing: parse_listing(t, directive);            break;
			case Token::Type::DirectiveUpload: parse_upload(t, directive);              break;
			case Token::Type::DirectiveCgi: parse_cgi(t, directive);                    break;
			case Token::Type::DirectiveRedirect: parse_redirect(t, directive);          break;

		default: 
			throw std::runtime_error("Invalid6\n");
	}
}

void ConfigParser::parse_listener(Token& t, Directive& directive)
{
	expect(t, Token::Type::DirectiveListen);

	const Token listen_tok = t;
	const Token interface_tok = m_lexer.advance();
	size_t colon_pos = interface_tok.value.find(':');
	if (colon_pos == std::string::npos)
		throw std::runtime_error("Invalid7\n");

	std::string host = interface_tok.value.substr(0, colon_pos);
	std::string port_str = interface_tok.value.substr(colon_pos + 1);

	std::vector<std::string> octets = Utils::split_string(host, '.');
	if (octets.size() != 4)
		throw std::runtime_error("Invalid8\n");
	for (size_t i = 0; i < octets.size(); i++)
	{
		size_t octet = std::atoi(octets.at(i).c_str());
		if (octet < 0 || octet > 255)
			throw std::runtime_error("Invalid9\n");
	}

	size_t port = std::atoi(port_str.c_str());
	if (port < 1 || port > 65535)
		throw std::runtime_error("Invalid10\n");

	directive.start_pos = listen_tok.start_pos;
	directive.name = listen_tok.value;
	directive.listen.host = host;
	directive.listen.port = port;

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_server_name(Token& t, Directive& directive)
{
	//@TODO debug disto!
	expect(t, Token::Type::DirectiveServerName);

	const Token server_name_tok = t;
	const Token server_name_arg_tok = m_lexer.advance();

	directive.name = server_name_tok.value;
	directive.start_pos = server_name_tok.start_pos;
	directive.args.push_back(server_name_arg_tok.value); 

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_upload_dir(Token& t, Directive& directive)
{
	const Token upload_dir_tok = t;
	expect(t, Token::Type::DirectiveUploadDir);

	const Token upload_dir_arg_tok = m_lexer.advance();
	expect(upload_dir_arg_tok, '/', upload_dir_arg_tok.value.size() - 1);

	directive.name = upload_dir_tok.value;
	directive.start_pos = upload_dir_tok.start_pos;
	directive.type = Token::Type::DirectiveUploadDir;
	directive.args.push_back(upload_dir_arg_tok.value); 

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_max_body_size(Token& t, Directive& directive)
{
	expect(t, Token::Type::DirectiveMaxBodySize);

	const Token max_body_size_tok = t;
	const Token max_body_size_num_tok = m_lexer.advance();

	if (!Utils::str_isdigit(max_body_size_num_tok.value))
		throw std::runtime_error("Invalid11\n");

	directive.args.push_back(max_body_size_num_tok.value); 

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_error_page(Token& t, Directive& directive)
{
	expect(t, Token::Type::DirectiveErrorPage);
	const Token error_page_tok = t;

	const Token error_page_code_tok = m_lexer.advance();
	if (!Utils::str_isdigit(error_page_code_tok.value))
		throw std::runtime_error("Invalid12\n");
	size_t code = std::atoi(error_page_code_tok.value.c_str());
	if (code < 100 || code > 599)
		throw std::runtime_error("Invalid13\n");

	const Token error_page_file_tok = m_lexer.advance();

	directive.name = error_page_tok.value;
	directive.start_pos = error_page_tok.start_pos;
	directive.type = Token::Type::DirectiveErrorPage;
	directive.args.push_back(error_page_code_tok.value);
	directive.args.push_back(error_page_file_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_root(Token& t, Directive& directive)
{
	const Token root_tok = t;
	expect(t, Token::Type::DirectiveRoot);

	const Token root_dir_tok = m_lexer.advance();
	expect(root_dir_tok, '/', root_dir_tok.value.size() - 1);

	directive.name = root_tok.value;
	directive.start_pos = root_tok.start_pos;
	directive.type = Token::Type::DirectiveRoot;
	directive.args.push_back(root_dir_tok.value); 

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_methods(Token& t, Directive& directive)
{
	const Token methods_tok = t;
	expect(t, Token::Type::DirectiveMethods);
	t = m_lexer.advance();

	directive.name = methods_tok.value;
	directive.start_pos = methods_tok.start_pos;
	directive.type = Token::Type::DirectiveMethods;

	while (t.type != Token::Type::Semicolon && t.type != Token::Type::Eof)
	{
		directive.args.push_back(t.value); 
		t = m_lexer.advance();
	}

	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_default_file(Token& t, Directive& directive)
{
	const Token default_file_tok = t;
	expect(t, Token::Type::DirectiveDefaultFile);

	const Token default_file_file_tok = m_lexer.advance();

	directive.name = default_file_tok.value;
	directive.start_pos = default_file_tok.start_pos;
	directive.type = Token::Type::DirectiveDefaultFile;
	directive.args.push_back(default_file_file_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_listing(Token& t, Directive& directive)
{
	const Token listing_tok = t;
	expect(t, Token::Type::DirectiveListing);

	const Token listing_bool_tok = m_lexer.advance();
	if (listing_bool_tok.value != "on" && listing_bool_tok.value != "off")
		throw std::runtime_error("Invalid14\n");

	directive.name = listing_tok.value;
	directive.start_pos = listing_tok.start_pos;
	directive.type = Token::Type::DirectiveListing;
	directive.args.push_back(listing_bool_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_upload(Token& t, Directive& directive)
{
	const Token upload_tok = t;
	expect(t, Token::Type::DirectiveUpload);

	const Token upload_bool_tok = m_lexer.advance();
	if (upload_bool_tok.value != "on" && upload_bool_tok.value != "off")
		throw std::runtime_error("Invalid15\n");

	directive.name = upload_tok.value;
	directive.start_pos = upload_tok.start_pos;
	directive.type = Token::Type::DirectiveUpload;
	directive.args.push_back(upload_bool_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_cgi(Token& t, Directive& directive)
{
	const Token cgi_tok = t;
	expect(t, Token::Type::DirectiveCgi);

	const Token cgi_extension_tok = m_lexer.advance();
	if (!ConfigKeywords::cgi_extensions.count(cgi_extension_tok.value))
		throw std::runtime_error("Invalid16\n");

	const Token cgi_file_tok = m_lexer.advance();

	directive.name = cgi_tok.value;
	directive.start_pos = cgi_tok.start_pos;
	directive.type = Token::Type::DirectiveCgi;
	directive.args.push_back(cgi_extension_tok.value);
	directive.args.push_back(cgi_file_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_redirect(Token& t, Directive& directive)
{
	const Token redirect_tok = t;
	expect(t, Token::Type::DirectiveRedirect);

	const Token redirect_code_tok = m_lexer.advance();
	if (!Utils::str_isdigit(redirect_code_tok.value))
		throw std::runtime_error("Invalid17\n");
	size_t code = std::atoi(redirect_code_tok.value.c_str());
	if (code < 300 || code > 399)
		throw std::runtime_error("Invalid18\n");

	const Token redirect_path_tok = m_lexer.advance();

	directive.name = redirect_tok.value;
	directive.start_pos = redirect_tok.start_pos;
	directive.type = Token::Type::DirectiveRedirect;
	directive.args.push_back(redirect_code_tok.value);
	directive.args.push_back(redirect_path_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Type::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::expect(const Token& t, Token::Type expected)
{
	if (t.type != expected)
	{
		throw std::runtime_error("Error - expected: " + token_type_to_literal(expected) + ", found: " + t.to_literal() + " - str: " + t.value);
	}
}

void ConfigParser::expect(const Token& t, char expected, size_t pos)
{
	if (pos > t.value.size() || expected != t.value.at(pos))
	{
		throw std::runtime_error("Error - expected: " + std::string(1, expected) + ", found: " + t.to_literal() + " - str: " + t.value);
	}
}

void ConfigParser::expect(const Token& t, Token::Type expected_type, char expected_ch, size_t pos)
{
	if (pos > t.value.size() ||  expected_ch != t.value.at(pos) || expected_type != t.type)
	{
		throw std::runtime_error("Error - expected: " + token_type_to_literal(expected_type) + ", found: " + t.to_literal() + " - str: " + t.value);
	}
}
