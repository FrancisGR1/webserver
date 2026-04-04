#include <cstdlib>
#include <string>

#include "core/Logger.hpp"

namespace test_utils
{
int diff_and_log(const char* file1_path, const char* file2_path, const char* log_file)
{
    std::string cmd = "diff " + std::string(file1_path) + " " + file2_path + " > " + log_file;
    Logger::debug("Diff: '%s'", cmd.c_str());
    return system(cmd.c_str());
}
}
