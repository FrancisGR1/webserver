#include "core/constants.hpp"
#include "core/Logger.hpp"
#include "core/Timer.hpp"

#include <string>

namespace constants
{
// server
const char* server_name = "webserv/1.0";
const size_t read_chunk_size = 4096;
const size_t pipe_buffer_capacity = 1048576;
const size_t write_chunk_size = 4096;
// - events
const size_t max_events = 512;
const Milliseconds epoll_timeout = 60000; // - connections
const size_t max_connections = 950;

// - timeouts
// -- connection
const Seconds idle_connection_timeout = 30;
// -- cgi
const Seconds cgi_timeout = 60.0;
const size_t cgi_max_failed_reads = 5;
const size_t cgi_max_output = 100000;
const std::string extensions[] = {".py", ".php", ".lua", ".js", ".sh", ".bla", ".cgi"};
const size_t extensions_num = sizeof(extensions) / sizeof(extensions[0]);
//  - configuration
const char* default_conf = "config/default.conf";
// - http response
const char* server_http_version = "HTTP/1.0";

// http
// CRLF
const char* crlf = "\r\n";
const char* crlfcrlf = "\r\n\r\n";
// - request
const size_t max_body_size = 1000000;
const size_t max_uri_size = 10000;
const char* body_whitespaces = " \t\n\f\v";

// color codes
const char* cyan = "\033[36m";
const char* blue = "\033[34m";
const char* green = "\033[32m";
const char* yellow = "\033[33m";
const char* red = "\033[31m";
const char* bright_red = "\033[91m";
const char* reset = "\033[0m";

// - logging
const Log::Level log_level = Log::Trace;

} // namespace constants
