#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

struct Token
{
    enum Type
    {
        // lexer types
        Word,
        Semicolon,
        LBrace,
        RBrace,
        Eof,
        Unknown,

        // block types
        NoBlock,
        Service,
        Location,

        // directive statement parts
        Directive,
        DirectiveName,
        DirectiveParameter,

        // specify the directive type further
        DirectiveListen,
        DirectiveServerName,
        DirectiveMaxBodySize,
        DirectiveErrorPage,
        DirectiveRoot,
        DirectiveMethods,
        DirectiveDefaultFile,
        DirectiveListing,
        DirectiveUpload,
        DirectiveUploadDir,
        DirectiveCgi,
        DirectiveRedirect
    };

    Token(Token::Type type, std::string value, size_t start_pos);
    Token(std::string value, size_t start_pos);

    // classifies its type
    Token::Type classify();
    void classify_word_in_context(Token::Type context);
    void classify_block();
    void classify_directive();

    std::string to_string() const;
    std::string to_literal() const;

    std::string value;
    Token::Type type;
    size_t start_pos;
};

std::ostream& operator<<(std::ostream& os, const Token& t);
std::string token_type_to_literal(Token::Type type);

#endif // TOKEN_HPP
