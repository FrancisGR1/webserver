#include "core/utils.hpp"
#include "Config.hpp"
#include "ConfigLexer.hpp"
#include "ConfigParser.hpp"

// ==============================================================
// Config class
// ==============================================================

Config::Config() {}

Config::Config(const char *file_path)
{
	//@TODO: criar func em utils: file_to_str
	m_file_content = utils::file_to_str(file_path);
}

//@TODO: colocar isto dentro do construtor?
void Config::load()
{
	ConfigLexer lexer(m_file_content);
	ConfigParser parser(lexer);

	const Config& parsed = parser.get();
	services = parsed.services;
}

std::ostream& operator<<(std::ostream& os, const Config& cfg)
{
	for (size_t i = 0; i < cfg.services.size(); i++)
	{
		os << cfg.services.at(i) << "\n";
	}
	return os;
}
