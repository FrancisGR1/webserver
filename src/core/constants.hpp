#ifndef CONSTANTS_HPP
# define CONSTANTS_HPP

#include <cstddef>
#include <string>

namespace constants 
{
	// colors
	extern const char* cyan;
	extern const char* blue;
	extern const char* green;
	extern const char* yellow;
	extern const char* red;
	extern const char* bright_red;
	extern const char* reset;

	// server
	extern const char* server_name;
	extern const char* default_conf;
	extern const char* server_http_version;
	extern const std::string py_ext;
	extern const size_t read_chunk_size;
	extern const size_t write_chunk_size;

	// http body
	extern const size_t max_body_size;
	extern const size_t max_uri_size;
	extern const char* body_whitespaces;

} // namespace constants

# endif // CONSTANTS_HPP
