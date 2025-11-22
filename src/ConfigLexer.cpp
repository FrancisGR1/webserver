#include "webserver.hpp"

//@QUESTION: não há melhor forma de organizar isto do que ser free func?
std::string token_type_to_string(Token::Type type) 
{
	switch(type) 
	{
		case Token::Type::Service:   return "Service";
		case Token::Type::Location:  return "Location";
		case Token::Type::Directive: return "Directive";
		case Token::Type::Parameter: return "Parameter";
		case Token::Type::Semicolon: return "Semicolon";
		case Token::Type::LBrace:    return "LBrace";
		case Token::Type::RBrace:    return "RBrace";
		case Token::Type::Eof:       return "Eof";
		default:                     return "Invalid";
	}
}

std::ostream& operator<<(std::ostream& os, const Token& t)
{
	//@TODO: alinhar
	os << std::left               
	   << std::setw(12)   
	   << token_type_to_string(t.type)
	   << " -> "
	   << t.value;
	return os;
};

ConfigLexer::ConfigLexer(const std::string& file_content)
{
	std::string build_string = ""; 

	Logger::trace("Lexer start");
	Logger::trace(file_content);
	size_t idx = 0;
	while (idx < file_content.size())
	{
		char c = file_content.at(idx);

		if (std::isspace(c))
		{
			tokenize(build_string);
			idx++;
		}
		else if  (c == '{' || c == '}' || c == ';')
		{
			tokenize(build_string);

			std::string char_to_str(1, c);
			tokenize(char_to_str);
			idx++;
		}
		else if (c == '#')
		{
			// skips the comment
			while (idx < file_content.size() && file_content.at(idx) != '\n')
				idx++;
		}
		else
		{
			build_string += c;
			idx++;
		}
	}
	tokenize(build_string);
	m_tokens.push_back(Token(Token::Type::Eof, "Eof"));
}

void ConfigLexer::tokenize(std::string& str)
{
	if (str.empty())
		return ;

	Token::Type type = classify_token(str);
	m_tokens.push_back(Token(type, str));

	str.clear();
}

Token::Type ConfigLexer::classify_token(const std::string& str)
{
	static const std::string server   = "service";
	static const std::string location = "location";
	static const std::set<std::string> delimiters  = { "{", "}", ";" };

	if (is_directive(str))
	{
		return Token::Type::Directive;
	}
	else if (str == server)
	{
		return Token::Type::Service;
	}
	else if (str == location)
	{
		return Token::Type::Location;
	}
	else if (delimiters.count(str))
	{
		if (str == "{")
			return Token::Type::LBrace;
		if (str == "}")
			return Token::Type::RBrace;
		if (str == ";")
			return Token::Type::Semicolon;
	}
	else
	{
		return Token::Type::Parameter;
	}
	return Token::Type::Invalid;
}

bool ConfigLexer::is_directive(const std::string& str)
{
	static const std::set<std::string> directive = 
	{ 
		"listen", "server_name", "max_body_size", 
		"error_page", "root", "methods", 
		"default_file", "listing", "upload", 
		"upload_dir", "cgi", "redirect" 
	};

	bool found = directive.count(str) > 0;
	return (found);
}


std::ostream& operator<<(std::ostream& os, const ConfigLexer& lexer)
{
	for (size_t i = 0; i < lexer.m_tokens.size(); i++)
	{
		os << "'" << i << "' " << lexer.m_tokens[i] << std::endl;
	}
	return os;
}

