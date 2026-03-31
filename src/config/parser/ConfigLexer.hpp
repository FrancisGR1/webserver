#ifndef CONFIGLEXER_HPP
#define CONFIGLEXER_HPP
#include "Token.hpp"
#include <ostream>
#include <string>
#include <vector>

class ConfigLexer
{
  public:
    ConfigLexer(const std::string& file_content);

    const Token advance();

    friend std::ostream& operator<<(std::ostream& os, const ConfigLexer& lexer);

  private:
    std::vector<Token> m_tokens;
    size_t m_token_idx;

    void tokenize(std::string& str, size_t start_pos);
    bool is_directive(const std::string& str);
};

#endif // CONFIGLEXER_HPP
