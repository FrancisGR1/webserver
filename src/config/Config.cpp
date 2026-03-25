#include "core/utils.hpp"
#include "core/Logger.hpp"
#include "config/parser/ConfigLexer.hpp"
#include "config/parser/ConfigParser.hpp"
#include "config/Config.hpp"

// ==============================================================
// Config class
// ==============================================================

Config::Config() {}

void Config::load(const char *file_path)
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
