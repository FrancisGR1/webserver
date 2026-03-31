#include "ConfigTypes.hpp"
#include "core/constants.hpp"
#include <cstdlib>
#include <iomanip>

// ==============================================================
// Keywords namespace
// ==============================================================

namespace ConfigKeywords
{
const std::set<std::string> make_delimiters()
{
    std::set<std::string> s;
    s.insert("{");
    s.insert("}");
    s.insert(";");
    return s;
}

const std::set<std::string> make_block()
{
    std::set<std::string> s;
    s.insert("service");
    s.insert("location");
    return s;
}

const std::set<std::string> make_directive()
{
    std::set<std::string> s;
    s.insert("listen");
    s.insert("server_name");
    s.insert("max_body_size");
    s.insert("error_page");
    s.insert("root");
    s.insert("methods");
    s.insert("default_file");
    s.insert("listing");
    s.insert("upload");
    s.insert("upload_dir");
    s.insert("cgi");
    s.insert("redirect");
    return s;
}

const std::set<std::string> make_cgi_extensions()
{
    std::set<std::string> s;
    s.insert(".py");
    // Add extensions here
    return s;
}
}
