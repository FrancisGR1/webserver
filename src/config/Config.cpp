#include "config/Config.hpp"
#include "config/parser/ConfigLexer.hpp"
#include "config/parser/ConfigParser.hpp"
#include "core/Logger.hpp"
#include "core/utils.hpp"

// ==============================================================
// Config class
// ==============================================================

Config::Config()
{
}

void Config::expect_unique_listeners(void) const
{
    std::set<std::string> seen;
    for (size_t i = 0; i < services.size(); i++)
    {
        for (size_t j = 0; j < services[i].listeners.size(); j++)
        {
            std::string key = services[i].listeners[j].host + ":" + services[i].listeners[j].port;
            if (seen.count(key))
            {
                throw std::runtime_error(
                    utils::fmt("Duplicate listener '%s' in config. Cannot continue.", key.c_str()));
            }
            seen.insert(key);
        }
    }
}

void Config::load(const char* file_path)
{
    Logger::info("Config: load '%s'", file_path);

    // load content into str memory
    m_file_content = utils::file_to_str(file_path);

    // parse
    ConfigLexer lexer(m_file_content);
    ConfigParser parser(lexer);

    // extract
    const Config& parsed = parser.get();
    services = parsed.services;

    expect_unique_listeners();

    Logger::debug_obj(*this, "Config: ");
}

std::ostream& operator<<(std::ostream& os, const Config& cfg)
{
    for (size_t i = 0; i < cfg.services.size(); i++)
    {
        os << cfg.services.at(i) << "\n";
    }
    return os;
}
