#include "utils.hpp"
#include <sstream>

namespace utils
{
	std::vector<std::string> split_string(const std::string& str, char delimiter)
	{
		std::vector<std::string> split;
		
		size_t start = 0;
		while (true)
		{
			size_t delimiter_pos = str.find(delimiter, start);
			if (delimiter_pos == std::string::npos)
			{
				split.push_back(str.substr(start));
				break ;
			}
			else if (delimiter_pos == start)
			{
				start++;
			}
			else
			{
				split.push_back(str.substr(start, delimiter_pos - start));
				start = delimiter_pos + 1;
			}
		}
		return split;
	}

	void str_trim_sides(std::string& str, const char *pattern)
	{
		size_t start = str.find_first_not_of(pattern);
		if (start == std::string::npos)
			return;

		size_t end = str.find_last_not_of(pattern);
		if (end == std::string::npos)
			return;

		if (start > 0)
			str.erase(0, start);
		if (end + 1 < str.size())
			str.erase(end + 1);
	}

	bool str_isdigit(const std::string& str)
	{
		for (size_t i = 0; i < str.size(); i++)
		{
			if (!std::isdigit(static_cast<unsigned char>(str.at(i))))
				return false;
		}
		return true;
	}

	void str_tolower(std::string& str)
	{
		for (size_t i = 0; i < str.size(); i++)
		{
			str.at(i) = std::tolower((unsigned char)str.at(i));
		}
	}

	int str_tohexadecimal(const std::string& str)
	{
		int result = 0;
		std::stringstream ss;

		ss << std::hex << str;
		if (!ss.fail())
			ss >> result;
		return result;
	}
} // namespace utils
