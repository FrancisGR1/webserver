#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#include "config/types/LocationConfig.hpp"
#include "config/types/ServiceConfig.hpp"
#include "core/Logger.hpp"
#include "core/Path.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"
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
    std::unique_ptr<Socket> socket;
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
    : title(title)
    , request(req)
    , file_path(file_path)
    , expected(expected)
{
    service = sv;
    location = lc;
    socket = std::make_unique<Socket>(3, service); // 3 = dummy fd
    events = std::make_unique<EventManager>();
    ctx = std::make_unique<RequestContext>(*socket, *events, service);

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

} // namespace tu
