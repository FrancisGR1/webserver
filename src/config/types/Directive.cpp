#include <string>
#include <vector>

#include "Directive.hpp"

Directive::Directive()
	: type(Token::Unknown) {} //@TODO 500

Directive::Directive(Token::Type t, std::vector<std::string> args)
	: type(t)
	, args(args) {}
	
