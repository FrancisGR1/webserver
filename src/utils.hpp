#ifndef UTILS_HPP
# define UTILS_HPP

#include <vector>
#include <string>

namespace utils
{
	std::vector<std::string> split_string(const std::string& str, char delimiter);
	void str_trim_sides(std::string& str, const char *pattern);
	bool str_isdigit(const std::string& str);
	void str_tolower(std::string& str);
	int str_tohexadecimal(const std::string& str);

	template<typename MapT>
	bool map_contains(const MapT& map, const typename MapT::key_type& key)
	{
		return map.find(key) != map.end();
	}
} // namespace utils

#endif // UTILS_HPP
