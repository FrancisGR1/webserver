#include <cstdarg>
#include <ctime>
#include <fstream>
#include <sstream>

#include "constants.hpp"
#include "utils.hpp"

namespace utils
{
std::vector<std::string> str_split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> split;

    if (delimiter.empty())
        return split;

    size_t start = 0;
    while (true)
    {
        size_t delimiter_pos = str.find(delimiter, start);
        if (delimiter_pos == std::string::npos)
        {
            split.push_back(str.substr(start));
            break;
        }
        else if (delimiter_pos == start)
        {
            start += delimiter.size();
        }
        else
        {
            split.push_back(str.substr(start, delimiter_pos - start));
            start = delimiter_pos + delimiter.size();
        }
    }
    return split;
}

void str_trim_sides(std::string& str, const char* pattern)
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

bool str_isdigit(const std::string& str, bool skip_spaces)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        unsigned char c = static_cast<unsigned char>(str.at(i));
        if (!std::isdigit(c))
        {
            if (!std::isspace(c))
                return false;
            if (std::isspace(c) && !skip_spaces)
                return false;
        }
    }
    return true;
}

std::string str_tolower(const std::string& str)
{
    std::string lowered = "";

    for (size_t i = 0; i < str.size(); i++)
    {
        lowered += std::tolower((unsigned char)str.at(i));
    }

    return lowered;
}

long long str_tohexadecimal(const std::string& str)
{
    long result = 0;
    std::stringstream ss;

    ss << std::hex << str;
    if (!ss.fail())
        ss >> result;
    return result;
}

std::string file_to_str(const std::string file_path)
{
    return file_to_str(file_path.c_str());
}

std::string file_to_str(const char* file_path)
{
    std::ifstream file(file_path);
    if (!file)
        throw std::runtime_error("Cannot open: " + std::string(file_path));
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string join_paths(const std::string& left, const std::string& right)
{
    if (left.empty())
        return right;
    if (right.empty())
        return left;
    char left_back = left.at(left.size() - 1);
    char right_front = right.at(0);
    if (left_back == '/' && right_front == '/')
        return left + right.substr(1);
    if (left_back != '/' && right_front != '/')
        return left + "/" + right;
    return left + right;
}

std::string fmt(const char* fmt, ...)
{
    //@TODO substituir por constant?
    char buffer[5000];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

std::string map_to_str(std::map<std::string, std::string> headers)
{
    std::string str;

    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        str += it->first + ": " + it->second + constants::crlf;
    }

    return str;
}

std::string http_date()
{
    char buf[63];
    std::time_t now = std::time(NULL);
    std::tm gmt;

    gmtime_r(&now, &gmt);

    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &gmt);

    return std::string(buf);
}

} // namespace utils
