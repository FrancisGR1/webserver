#ifndef DIRECTIVE_HPP
# define DIRECTIVE_HPP

#include <string>
#include <vector>

#include "config/types/Listener.hpp"
#include "config/parser/Token.hpp"

struct Directive
{
	std::string name;
	Token::Type type;
	size_t start_pos;

	// a directive has either one of these
	Listener listen;
	std::vector<std::string> args;
};

#endif // DIRECTIVE_HPP
