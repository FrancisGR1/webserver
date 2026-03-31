#include "ConfigLexer.hpp"
#include <cctype>

// ==============================================================
// ConfigLexer class
// ==============================================================

ConfigLexer::ConfigLexer(const std::string& file_content)
    : m_token_idx(0)
{
    std::string build_string = "";

    size_t idx = 0;
    while (idx < file_content.size())
    {
        char c = file_content.at(idx);

        if (std::isspace(c))
        {
            tokenize(build_string, idx);
            idx++;
        }
        else if (c == '{' || c == '}' || c == ';')
        {
            tokenize(build_string, idx);

            std::string char_to_str(1, c);
            tokenize(char_to_str, idx);
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
    tokenize(build_string, idx);
    m_tokens.push_back(Token(Token::Eof, "Eof", idx));
}

const Token ConfigLexer::advance()
{
    if (m_token_idx >= m_tokens.size())
        return m_tokens[m_token_idx];
    else
    {
        const Token tok = m_tokens[m_token_idx];
        m_token_idx++;
        return tok;
    }
}

void ConfigLexer::tokenize(std::string& str, size_t start_pos)
{
    if (str.empty())
        return;

    Token tok(str, start_pos);
    m_tokens.push_back(tok);

    str.clear();
}

std::ostream& operator<<(std::ostream& os, const ConfigLexer& lexer)
{
    for (size_t i = 0; i < lexer.m_tokens.size(); i++)
    {
        os << "'" << i << "' " << lexer.m_tokens[i] << "\n";
    }
    return os;
}
