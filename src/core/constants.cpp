#include "constants.hpp"

namespace constants
{
    // color codes
    const char* cyan = "\033[36m";
    const char* blue = "\033[34m";
    const char* green = "\033[32m";
    const char* yellow = "\033[33m";
    const char* red = "\033[31m";
    const char* bright_red = "\033[91m";
    const char* reset = "\033[0m";

    // server
    //
    // - configuration
    const char* default_conf = "config/default.conf";
    // - http response
    const char* server_http_version = "HTTP/1.0";
    const std::string py_ext = "py";

    // - http request
    const size_t max_body_size = 1000000;
    const size_t max_uri_size  = 10000;
    const char* body_whitespaces = " \t\n\f\v";


} // namespace constants
