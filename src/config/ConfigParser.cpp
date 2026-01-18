#include <string>
#include <ostream>
#include <cstdlib>

#include "core/utils.hpp"
#include "ConfigTypes.hpp"
#include "ConfigLexer.hpp"
#include "ConfigParser.hpp"
#include "Config.hpp"
#include "http/StatusCode.hpp"

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
	while (t.type != Token::Eof)
	{
		t.classify_word_in_context(Token::NoBlock);
		switch (t.type)
		{
			case Token::Service:
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
	expect(t, Token::Service);
	t = m_lexer.advance();
	expect(t, Token::LBrace);
	t = m_lexer.advance();

	while (t.type != Token::RBrace && t.type != Token::Eof)
	{
		t.classify_word_in_context(Token::Service);

		switch(t.type)
		{
			case Token::Location:
				{
					LocationConfig location;
					parse_location(t, location);
					service.set(location);
					break;
				}
			case Token::DirectiveName:
				{
					Directive directive;
					parse_directive(t, Token::Service, directive);
					service.set(directive);
					break;
				}
			default:
				{
					throw std::runtime_error("Error: Invalid2\n");
				}
		}
	}

	expect(t, Token::RBrace);
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
	expect(t, Token::Location);
	const Token token_dir = m_lexer.advance();
	location.name = parse_location_dir(token_dir);
	t = m_lexer.advance();
	expect(t, Token::LBrace);
	t = m_lexer.advance();

	while (t.type != Token::RBrace && t.type != Token::Eof)
	{
		t.classify_word_in_context(Token::Location);
		Directive directive;
		parse_directive(t, Token::Location, directive);
		location.set(directive);
	}

	expect(t, Token::RBrace);
	t = m_lexer.advance();
}

void ConfigParser::parse_directive(Token& t, Token::Type context, Directive& directive)
{
	expect(t, Token::DirectiveName);
	directive.name = t.value;

	t.classify_word_in_context(Token::DirectiveName);
	directive.type = t.type;

	//@TODO: colocar aqui um check de contexto em vez dos throws abaixo
	switch(t.type)
	{
		case Token::DirectiveListen: 
			if (context != Token::Service)
				throw std::runtime_error("Invalid3\n");
			parse_listener(t, directive);
			break;
		case Token::DirectiveServerName: 
			if (context != Token::Service)
				throw std::runtime_error("Invalid4\n");
			parse_server_name(t, directive);
			break;

		case Token::DirectiveUploadDir:   parse_upload_dir(t, directive);     break;
		case Token::DirectiveMaxBodySize: parse_max_body_size(t, directive);  break;
		case Token::DirectiveErrorPage:   parse_error_page(t, directive);     break;
		case Token::DirectiveRoot:        parse_root(t, directive);           break;
		case Token::DirectiveMethods:     parse_methods(t, directive);        break;
		case Token::DirectiveDefaultFile: parse_default_file(t, directive);   break;
		case Token::DirectiveListing:     parse_listing(t, directive);        break;
		case Token::DirectiveUpload:      parse_upload(t, directive);         break;
		case Token::DirectiveCgi:         parse_cgi(t, directive);            break;
		case Token::DirectiveRedirect:    parse_redirect(t, directive);       break;

		default: 
			throw std::runtime_error("Invalid6\n");
	}
}

void ConfigParser::parse_listener(Token& t, Directive& directive)
{
	expect(t, Token::DirectiveListen);

	const Token listen_tok = t;
	const Token interface_tok = m_lexer.advance();
	size_t colon_pos = interface_tok.value.find(':');
	if (colon_pos == std::string::npos)
		throw std::runtime_error("Invalid7\n");

	std::string host = interface_tok.value.substr(0, colon_pos);
	std::string port_str = interface_tok.value.substr(colon_pos + 1);

	std::vector<std::string> octets = utils::str_split(host, '.');
	if (octets.size() != 4)
		throw std::runtime_error("Invalid8\n");
	for (size_t i = 0; i < octets.size(); i++)
	{
		size_t octet = atoi(octets.at(i).c_str());
		if (octet > 255)
			throw std::runtime_error("Invalid9\n");
	}

	size_t port = atoi(port_str.c_str());
	if (port < 1 || port > 65535)
		throw std::runtime_error("Invalid10\n");

	directive.start_pos = listen_tok.start_pos;
	directive.name = listen_tok.value;
	directive.listen.host = host;
	directive.listen.port = port;

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_server_name(Token& t, Directive& directive)
{
	//@TODO debug disto!
	expect(t, Token::DirectiveServerName);

	const Token server_name_tok = t;
	const Token server_name_arg_tok = m_lexer.advance();

	directive.name = server_name_tok.value;
	directive.start_pos = server_name_tok.start_pos;
	directive.args.push_back(server_name_arg_tok.value); 

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_upload_dir(Token& t, Directive& directive)
{
	const Token upload_dir_tok = t;
	expect(t, Token::DirectiveUploadDir);

	const Token upload_dir_arg_tok = m_lexer.advance();
	expect(upload_dir_arg_tok, '/', upload_dir_arg_tok.value.size() - 1);

	directive.name = upload_dir_tok.value;
	directive.start_pos = upload_dir_tok.start_pos;
	directive.type = Token::DirectiveUploadDir;
	directive.args.push_back(upload_dir_arg_tok.value); 

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_max_body_size(Token& t, Directive& directive)
{
	expect(t, Token::DirectiveMaxBodySize);

	const Token max_body_size_tok = t;
	const Token max_body_size_num_tok = m_lexer.advance();

	if (!utils::str_isdigit(max_body_size_num_tok.value))
		throw std::runtime_error("Invalid11\n");

	directive.args.push_back(max_body_size_num_tok.value); 

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_error_page(Token& t, Directive& directive)
{
	expect(t, Token::DirectiveErrorPage);
	const Token error_page_tok = t;

	const Token error_page_code_tok = m_lexer.advance();
	if (!utils::str_isdigit(error_page_code_tok.value))
		throw std::runtime_error("Invalid12\n");
	size_t code = atoi(error_page_code_tok.value.c_str());
	if (code < 100 || code > 599)
		throw std::runtime_error("Invalid13\n");

	const Token error_page_file_tok = m_lexer.advance();

	directive.name = error_page_tok.value;
	directive.start_pos = error_page_tok.start_pos;
	directive.type = Token::DirectiveErrorPage;
	directive.args.push_back(error_page_code_tok.value);
	directive.args.push_back(error_page_file_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_root(Token& t, Directive& directive)
{
	const Token root_tok = t;
	expect(t, Token::DirectiveRoot);

	const Token root_dir_tok = m_lexer.advance();
	expect(root_dir_tok, '/', root_dir_tok.value.size() - 1);

	directive.name = root_tok.value;
	directive.start_pos = root_tok.start_pos;
	directive.type = Token::DirectiveRoot;
	directive.args.push_back(root_dir_tok.value); 

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_methods(Token& t, Directive& directive)
{
	const Token methods_tok = t;
	expect(t, Token::DirectiveMethods);
	t = m_lexer.advance();

	directive.name = methods_tok.value;
	directive.start_pos = methods_tok.start_pos;
	directive.type = Token::DirectiveMethods;

	while (t.type != Token::Semicolon && t.type != Token::Eof)
	{
		directive.args.push_back(t.value); 
		t = m_lexer.advance();
	}

	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_default_file(Token& t, Directive& directive)
{
	const Token default_file_tok = t;
	expect(t, Token::DirectiveDefaultFile);

	const Token default_file_file_tok = m_lexer.advance();

	directive.name = default_file_tok.value;
	directive.start_pos = default_file_tok.start_pos;
	directive.type = Token::DirectiveDefaultFile;
	directive.args.push_back(default_file_file_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_listing(Token& t, Directive& directive)
{
	const Token listing_tok = t;
	expect(t, Token::DirectiveListing);

	const Token listing_bool_tok = m_lexer.advance();
	if (listing_bool_tok.value != "on" && listing_bool_tok.value != "off")
		throw std::runtime_error("Invalid14\n");

	directive.name = listing_tok.value;
	directive.start_pos = listing_tok.start_pos;
	directive.type = Token::DirectiveListing;
	directive.args.push_back(listing_bool_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_upload(Token& t, Directive& directive)
{
	const Token upload_tok = t;
	expect(t, Token::DirectiveUpload);

	const Token upload_bool_tok = m_lexer.advance();
	if (upload_bool_tok.value != "on" && upload_bool_tok.value != "off")
		throw std::runtime_error("Invalid15\n");

	directive.name = upload_tok.value;
	directive.start_pos = upload_tok.start_pos;
	directive.type = Token::DirectiveUpload;
	directive.args.push_back(upload_bool_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_cgi(Token& t, Directive& directive)
{
	const Token cgi_tok = t;
	expect(t, Token::DirectiveCgi);

	const Token cgi_extension_tok = m_lexer.advance();
	if (!ConfigKeywords::cgi_extensions.count(cgi_extension_tok.value))
		throw std::runtime_error("Invalid16\n");

	const Token cgi_file_tok = m_lexer.advance();

	directive.name = cgi_tok.value;
	directive.start_pos = cgi_tok.start_pos;
	directive.type = Token::DirectiveCgi;
	directive.args.push_back(cgi_extension_tok.value);
	directive.args.push_back(cgi_file_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
	t = m_lexer.advance();
}

void ConfigParser::parse_redirect(Token& t, Directive& directive)
{
	const Token redirect_tok = t;
	expect(t, Token::DirectiveRedirect);

	const Token redirect_code_tok = m_lexer.advance();
	if (!utils::str_isdigit(redirect_code_tok.value))
		throw std::runtime_error("Invalid17\n");
	size_t code = atoi(redirect_code_tok.value.c_str());
	if (!StatusCode::is_redirection(code))
		throw std::runtime_error("Invalid18\n");

	const Token redirect_path_tok = m_lexer.advance();

	directive.name = redirect_tok.value;
	directive.start_pos = redirect_tok.start_pos;
	directive.type = Token::DirectiveRedirect;
	directive.args.push_back(redirect_code_tok.value);
	directive.args.push_back(redirect_path_tok.value);

	t = m_lexer.advance();
	expect(t, Token::Semicolon);
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
