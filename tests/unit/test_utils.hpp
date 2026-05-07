#include <cstdlib>
#include <memory>
#include <string>

#include <sys/stat.h>

#include "config/types/LocationConfig.hpp"
#include "config/types/ServiceConfig.hpp"
#include "core/Logger.hpp"
#include "core/Path.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"
#include "server/ConnectionPool.hpp"
#include "server/EventManager.hpp"
#include "server/Socket.hpp"

namespace tu
{
struct HandlerTestCase
{
    HandlerTestCase(
        const std::string title,
        Path file_path,
        ServiceConfig sv,
        const LocationConfig& lc,
        Request req,
        Response expected);

    std::string title;
    Request request;
    Path file_path;
    ServiceConfig service;
    LocationConfig location;

    // owned resources
    Listener listener;
    std::unique_ptr<Socket> socket;
    std::unique_ptr<ConnectionPool> conns;
    std::unique_ptr<EventManager> events;
    std::unique_ptr<RequestContext> ctx;

    Response expected;
};

HandlerTestCase::HandlerTestCase(
    const std::string title,
    Path file_path,
    ServiceConfig sv,
    const LocationConfig& lc,
    Request req,
    Response expected)
    : title{title}
    , request{req}
    , file_path{file_path}
    , listener{"0", "0"}
    , expected{expected}
{
    service = sv;
    location = lc;
    socket = std::make_unique<Socket>(3, listener); // 3 = dummy fd
    conns = std::make_unique<ConnectionPool>(5);
    events = std::make_unique<EventManager>(1024, *conns);
    ctx = std::make_unique<RequestContext>(nullptr, service);

    ctx->config().set(file_path);
    ctx->config().set(location);
}

// compares two files by using 'diff' system cmd
// logs the output in log_file
int diff_and_log(const char* expected, const char* test, const char* log_file)
{
    std::string cmd = "diff " + std::string(expected) + " " + test + " > " + log_file;
    Logger::debug("Diff: '%s'", cmd.c_str());
    return system(cmd.c_str());
}

bool file_exists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool compare_bodies(const Response& result, const Response& expected)
{

    if (result.body_fd() > -1)
    // compare only the string body
    {
        std::string expected_substr = expected.body().substr(0, expected.body().size());
        return result.body() == expected_substr;
    }

    return result.body() == expected.body();
}

} // namespace tu
