#pragma once
#include "webserver.hpp"

struct Token
{
	enum Type
	{
		Service,
		Location,
		Directive,
		Parameter,
		Semicolon,
		LBrace,
		RBrace,
		Eof,
		Invalid
	};

	Token(Token::Type type, std::string value)
		: type(type)
		, value(value)
	{}


	Token::Type type;
	std::string value;
};

std::string token_type_to_string(Token::Type type);
std::ostream& operator<<(std::ostream& os, const Token& t);

class ConfigLexer
{
	public:
		ConfigLexer(const std::string& file_content);

		const Token& next_token() const;
		void print() const;
		friend std::ostream& operator<<(std::ostream& os, const ConfigLexer& lexer);


	private:
		std::vector<Token> m_tokens;

		void	tokenize(std::string& str);
		Token::Type classify_token(const std::string& str);
		bool	is_directive(const std::string& str);
};
