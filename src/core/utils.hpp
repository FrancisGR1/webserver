#ifndef UTILS_HPP
# define UTILS_HPP

#include <map>
#include <vector>
#include <string>
#include <sstream>

namespace utils
{
	std::vector<std::string> str_split(const std::string& str, const std::string& delimiter);
	void str_trim_sides(std::string& str, const char *pattern);
	bool str_isdigit(const std::string& str, bool skip_spaces = false);
	std::string str_tolower(const std::string& str);
	long long str_tohexadecimal(const std::string& str);
	std::string file_to_str(const char *file_path);
	std::string join_paths(const std::string& left, const std::string& right);
	std::string fmt(const char* fmt, ...);
	std::string map_to_str(std::map<std::string, std::string>headers);
	std::string http_date();

	// find element in container
	template<typename Container>
	bool contains(const Container& data, const typename Container::key_type& element)
	{
		return data.find(element) != data.end();
	}

	template<typename T>
	std::string to_string(const T& obj)
	{
		std::ostringstream oss;
		oss << obj;
		return oss.str();
	}

} // namespace util

#endif // UTILS_HPP
