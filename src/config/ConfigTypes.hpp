#ifndef CONFIGTYPES_HPP
# define CONFIGTYPES_HPP

#include <string>
#include <ostream>
#include <cstddef>
#include <set>

#include "http/StatusCode.hpp"

// ==============================================================
// Keywords namespace
// ==============================================================

namespace ConfigKeywords
{
    const std::set<std::string> make_delimiters();
    const std::set<std::string> make_block();
    const std::set<std::string> make_directive();
    const std::set<std::string> make_cgi_extensions();

    const std::set<std::string> delimiters = make_delimiters();
    const std::set<std::string> block = make_block();
    const std::set<std::string> directive = make_directive();
    const std::set<std::string> cgi_extensions = make_cgi_extensions();
}

// ==============================================================
// Token struct
// ==============================================================



// ==============================================================
// Config Types
// ==============================================================

#endif //CONFIGTYPES_HPP