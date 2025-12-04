#include "constants.hpp"

namespace constants
{
    // http body
    const size_t MAX_BODY_SIZE = 1000000;
    const char* BODY_WHITESPACES = " \t\n\f\v";

    // colors
    const char* CYAN = "\033[36m";
    const char* BLUE = "\033[34m";
    const char* GREEN = "\033[32m";
    const char* YELLOW = "\033[33m";
    const char* RED = "\033[31m";
    const char* BRIGHT_RED = "\033[91m";
    const char* RESET = "\033[0m";

} // namespace constants
