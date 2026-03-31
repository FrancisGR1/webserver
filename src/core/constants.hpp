#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstddef>
#include <string>

#include "Logger.hpp"
#include "Timer.hpp"

namespace constants
{
// server
// - config
extern const char* server_name;
extern const char* default_conf;
extern const char* server_http_version;
extern const std::string py_ext;
extern const size_t read_chunk_size;
extern const size_t read_pipe_chunk_size; // should match /proc/sys/fs/pipe-max-size
extern const size_t write_chunk_size;
// - timeouts
// -- connection
extern const Seconds connection_timeout;
// -- cgi
extern const Seconds cgi_timeout;
extern const size_t cgi_max_failed_reads;
extern const size_t cgi_max_output;

// http
// CRLF
extern const char* crlf;
extern const char* crlfcrlf;
// - body
extern const size_t max_body_size;
extern const size_t max_uri_size;
extern const char* body_whitespaces;

// colors
extern const char* cyan;
extern const char* blue;
extern const char* green;
extern const char* yellow;
extern const char* red;
extern const char* bright_red;
extern const char* reset;

// logging
extern const Log::Level log_level;

} // namespace constants

#endif // CONSTANTS_HPP
