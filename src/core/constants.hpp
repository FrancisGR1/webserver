#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstddef>
#include <string>

#include "core/Logger.hpp"
#include "core/Timer.hpp"

namespace constants
{
// server
// - config
extern const char* server_name;
extern const char* default_conf;
extern const char* server_http_version;
extern const size_t read_chunk_size;      // getconf PAGE_SIZE
extern const size_t pipe_buffer_capacity; // cat /proc/sys/fs/pipe-max-size
extern const size_t write_chunk_size;
// - events
extern const size_t max_events;
extern const Milliseconds epoll_timeout;
// - connections
extern const size_t max_connections; // check ulimit -n "open connections" - should be a little less than the limit.
                                     // e.g.: 1024 - 50 ~= 950
// - timeouts
// -- connection
extern const Seconds request_timeout;
extern const Seconds idle_connection_timeout;
// -- cgi
extern const Seconds cgi_timeout;
extern const size_t cgi_max_failed_reads;
extern const size_t cgi_max_output;
extern const std::string extensions[];
extern const size_t extensions_num;

// http
// CRLF
extern const char* crlf;
extern const char* crlfcrlf;
// - body
extern const size_t max_body_size;
extern const size_t max_uri_size;
extern const char* body_whitespaces;

// colors
extern const char* dim;
extern const char* cyan;
extern const char* blue;
extern const char* green;
extern const char* yellow;
extern const char* red;
extern const char* bright_red;
extern const char* reset;

// logging
extern const Log::Level log_level;

// - name codes
extern const char* conn;
extern const char* cgi;
extern const char* res;

} // namespace constants

#endif // CONSTANTS_HPP
