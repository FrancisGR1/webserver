#ifndef CONFIGLEXER_HPP
# define CONFIGLEXER_HPP
#include <string>
#include <ostream>
#include <vector>
#include "config/ConfigTypes.hpp"
#include "Token.hpp"

class ConfigLexer
{
	public:
		ConfigLexer(const std::string& file_content);

		const Token advance();

		friend std::ostream& operator<<(std::ostream& os, const ConfigLexer& lexer);


	private:
		std::vector<Token> m_tokens;
		size_t m_token_idx;

		void	tokenize(std::string& str, size_t start_pos);
		bool	is_directive(const std::string& str);
};

#endif //CONFIGLEXER_HPP
