#include "Configurations.hpp"

Configurations::Configurations(const std::string& file_path)
	: m_file_content(file_path)
{

	if (!m_file_content)
	{
		//@TODO: mandar erro
	}
};

void Configurations::parse()
{

}

std::ostream& operator<<(std::ostream& os, const Configurations& cfg)
{
    os << "@TODO: operator<< Configurations\n";
    return os;
}
