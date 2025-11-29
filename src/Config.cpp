#include <sstream>
#include <fstream>
#include "Config.hpp"
#include "ConfigLexer.hpp"
#include "ConfigParser.hpp"

// ==============================================================
// Config class
// ==============================================================

Config::Config() {}

Config::Config(const char *file_path)
{
	std::ifstream file(file_path);
	if (!file)
		throw std::runtime_error("Cannot open configurations' file");
	std::stringstream buffer;
	buffer << file.rdbuf();
	m_file_content = buffer.str();
}

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
