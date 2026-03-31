#include <iomanip>
#include <sstream>
#include <string>

#include "Token.hpp"

Token::Token(Token::Type type, std::string value, size_t start_pos)
    : value(value)
    , type(type)
    , start_pos(start_pos)
{
}

Token::Token(std::string value, size_t start_pos)
    : value(value)
    , type(classify())
    , start_pos(start_pos)
{
}

std::string Token::to_string() const
{
    switch (type)
    {
        case Token::Word: return "Word";
        case Token::Semicolon: return "Semicolon";
        case Token::LBrace: return "LBrace";
        case Token::RBrace: return "RBrace";
        case Token::Eof: return "Eof";

        case Token::Service: return "Service";
        case Token::Location: return "Location";
        case Token::Directive: return "Directive";
        case Token::DirectiveName: return "DirectiveName";
        case Token::DirectiveParameter: return "DirectiveParameter";

        default: return "Unknown";
    }
}

std::string Token::to_literal() const
{
    switch (type)
    {
        case Token::Word: return value;
        case Token::Semicolon: return ";";
        case Token::LBrace: return "{";
        case Token::RBrace: return "}";
        case Token::Eof: return "Eof";

        case Token::Service: return "Service";
        case Token::Location: return "Location";
        case Token::Directive: return "Directive";
        case Token::DirectiveName: return value;
        case Token::DirectiveParameter: return value;

        default: return "Unknown";
    }
}

std::string token_type_to_literal(Token::Type type)
{
    switch (type)
    {
        case Token::Word: return "word";
        case Token::Semicolon: return ";";
        case Token::LBrace: return "{";
        case Token::RBrace: return "}";
        case Token::Eof: return "Eof";
        case Token::Unknown: return "Unknown";

        // block types
        case Token::NoBlock: return "NoBlock";
        case Token::Service: return "service";
        case Token::Location: return "location";

        // directive statement parts
        case Token::Directive: return "Directive";
        case Token::DirectiveName: return "DirectiveName";
        case Token::DirectiveParameter: return "DirectiveParameter";

        // specify the directive type further
        case Token::DirectiveListen: return "DirectiveListen";
        case Token::DirectiveServerName: return "DirectiveServerName";
        case Token::DirectiveMaxBodySize: return "DirectiveMaxBodySize";
        case Token::DirectiveErrorPage: return "DirectiveErrorPage";
        case Token::DirectiveRoot: return "DirectiveRoot";
        case Token::DirectiveMethods: return "DirectiveMethods";
        case Token::DirectiveDefaultFile: return "DirectiveDefaultFile";
        case Token::DirectiveListing: return "DirectiveListing";
        case Token::DirectiveUpload: return "DirectiveUpload";
        case Token::DirectiveUploadDir: return "DirectiveUploadDir";
        case Token::DirectiveCgi: return "DirectiveCgi";
        case Token::DirectiveRedirect: return "DirectiveRedirect";
    }
    return "Unknown";
}

std::ostream& operator<<(std::ostream& os, const Token& t)
{
    os << std::left << std::setw(12) << t.to_string() << " -> " << t.value;
    return os;
};

Token::Type Token::classify()
{
    if (value == "{")
        return Token::LBrace;
    else if (value == "}")
        return Token::RBrace;
    else if (value == ";")
        return Token::Semicolon;
    else
        return Token::Word; // if its not a delimiter, it defaults to a word
}

void Token::classify_word_in_context(Token::Type context)
{
    switch (context)
    {
        case Token::NoBlock: type = Token::Service; break;
        case Token::Service:
            if (value == "location")
                type = Token::Location;
            else
                type = Token::DirectiveName;
            break;
        case Token::Location: type = Token::DirectiveName; break;
        case Token::DirectiveName: classify_directive(); break;
        default: type = Token::Unknown;
    }
}

void Token::classify_block()
{
    if (value == "service")
        type = Token::Service;
    else if (value == "location")
        type = Token::Location;
    else
        type = Token::Unknown;
}

void Token::classify_directive()
{
    if (value == "listen")
        type = Token::DirectiveListen;
    else if (value == "server_name")
        type = Token::DirectiveServerName;
    else if (value == "max_body_size")
        type = Token::DirectiveMaxBodySize;
    else if (value == "error_page")
        type = Token::DirectiveErrorPage;
    else if (value == "root")
        type = Token::DirectiveRoot;
    else if (value == "methods")
        type = Token::DirectiveMethods;
    else if (value == "default_file")
        type = Token::DirectiveDefaultFile;
    else if (value == "listing")
        type = Token::DirectiveListing;
    else if (value == "upload")
        type = Token::DirectiveUpload;
    else if (value == "upload_dir")
        type = Token::DirectiveUploadDir;
    else if (value == "cgi")
        type = Token::DirectiveCgi;
    else if (value == "return")
        type = Token::DirectiveRedirect;
    else
        type = Token::DirectiveParameter;
}
