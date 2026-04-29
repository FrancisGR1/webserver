#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config/types/LocationConfig.hpp"
#include "config/types/ServiceConfig.hpp"
#include "core/Logger.hpp"
#include "core/Path.hpp"
#include "http/StatusCode.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/response/ResponseError.hpp"
#include "server/Connection.hpp"
#include "server/EventManager.hpp"

#include "test_utils.hpp"

// how much time we should wait for idle subprocesses
// if the subprocess doesn't do anything for TEST_TIMEOUT time
// it gives a 504
const Seconds TEST_TIMEOUT = 6;

struct TestCase
{
    TestCase(const std::string title, Path script_path, Request req, Response expected)
        : title{title}
        , request{req}
        , script_path{script_path}
        , listener{"0", "0"}
        , expected{expected}
    {
        script_location = LocationConfig{"/scripts", "./test_data/"};
        service = ServiceConfig{{script_location}};
        socket = std::make_unique<Socket>(3, listener); // 3 = dummy fd
        events = std::make_unique<EventManager>(1024);
        conn = std::make_unique<Connection>(3, listener, service);
        ctx = std::make_unique<RequestContext>(conn.get(), service);

        // load cgi directives
        Directive directive{Token::DirectiveCgi, {".py", "/usr/bin/python3"}};
        script_location.set(directive);
        directive = {Token::DirectiveCgi, {".lua", "/usr/bin/lua"}};
        script_location.set(directive);
        directive = {Token::DirectiveCgi, {".php", "/usr/bin/php"}};
        script_location.set(directive);
        directive = {Token::DirectiveCgi, {".sh", "/usr/bin/sh"}};
        script_location.set(directive);

        // load allowed method
        directive = {Token::DirectiveMethods, {"GET"}};
        script_location.set(directive);

        ctx->config().set(script_path);
        ctx->config().set(script_location);
    }

    std::string title;
    Request request;
    Path script_path;
    LocationConfig script_location;
    ServiceConfig service;

    // Owned resources
    Listener listener;
    std::unique_ptr<Connection> conn;
    std::unique_ptr<Socket> socket;
    std::unique_ptr<EventManager> events;
    std::unique_ptr<RequestContext> ctx;

    Response expected;
};

std::vector<TestCase> generate_good_test_cases(void)
{
    std::vector<TestCase> test_cases = {};
    test_cases.reserve(100);
    // clang-format off

    // simple hello world
    test_cases.emplace_back(
        "simple script",
        "./test_data/scripts/good/hello.py",
        Request("GET", "/scripts/good/hello.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}}, "Hello, World.")
    );

    // empty body
    test_cases.emplace_back(
        "empty body",
        "./test_data/scripts/good/empty.py",
        Request("GET", "/scripts/good/empty.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}}, "")
    );

    // custom status code
    test_cases.emplace_back(
        "custom status code",
        "./test_data/scripts/good/created.py",
        Request("GET", "/scripts/good/created.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::Created, {{"Status", "201"}, {"Content-type", "text/html"}}, "Created.")
    );

    // multiple headers
    test_cases.emplace_back(
        "multiple headers",
        "./test_data/scripts/good/multi_headers.py",
        Request("GET", "/scripts/good/multi_headers.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}, {"X-Custom", "value"}}, "Multi.")
    );

    // large body
    test_cases.emplace_back(
        "large body",
        "./test_data/scripts/good/large.py",
        Request("GET", "/scripts/good/large.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}}, std::string(8192, 'A'))
    );
    
    // echo small body
    std::string small_body = "Hello World!";
    test_cases.emplace_back(
        "echo small body",
        "./test_data/scripts/good/echo_stdin.py",
        Request("GET", "/scripts/good/echo_stdin.py", "", "HTTP/1.1", {}, small_body, {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}}, small_body)
    );

    // echo big body
    // @NOTE because we don't have an event loop in the unit tests,
    // we test a safe big size (half of the max capacity), which is the theoretical
    // max for one big message.
    // Bigger messages should be tested in the integration tests
    std::string big_body(65536 / 2, 'A');
    test_cases.emplace_back(
        "echo big body",
        "./test_data/scripts/good/echo_stdin.py",
        Request("GET", "/scripts/good/echo_stdin.py", "", "HTTP/1.1", {}, big_body, {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}}, big_body)
    );

    // extra cgi scripts
    // php
    test_cases.emplace_back(
        "php",
        "./test_data/scripts/good/hello.php",
        Request("GET", "/scripts/good/hello.php", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}}, "Hello, World.")
    );

    // lua
    test_cases.emplace_back(
        "lua",
        "./test_data/scripts/good/hello.lua",
        Request("GET", "/scripts/good/hello.lua", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}}, "Hello, World.")
    );

    // sh
    test_cases.emplace_back(
        "sh",
        "./test_data/scripts/good/hello.sh",
        Request("GET", "/scripts/good/hello.sh", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::Ok, {{"Content-type", "text/html"}}, "Hello, World.")
    );

    // clang-format on
    return test_cases;
}

std::vector<TestCase> generate_bad_test_cases(void)
{
    std::vector<TestCase> test_cases = {};
    test_cases.reserve(100);
    //clang-format off
    // segfault - process killed by SIGSEGV
    //@TODO verificar segfault test isoladamente
    test_cases.emplace_back(
        "segfault",
        "./test_data/scripts/bad/segfault.py",
        Request("GET", "/scripts/bad/segfault.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::BadGateway, {}, ""));
    // abort - process killed by SIGABRT
    test_cases.emplace_back(
        "abort",
        "./test_data/scripts/bad/abort.py",
        Request("GET", "/scripts/bad/abort.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::BadGateway, {}, ""));
    // crash - unhandled exception, exit code 1
    test_cases.emplace_back(
        "crash",
        "./test_data/scripts/bad/crash.py",
        Request("GET", "/scripts/bad/crash.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::BadGateway, {}, ""));
    // non-zero exit code
    test_cases.emplace_back(
        "non zero exit code",
        "./test_data/scripts/bad/non_0_exit_code.py",
        Request("GET", "/scripts/bad/non_0_exit_code.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::BadGateway, {}, ""));
    // infinite loop - timeout
    test_cases.emplace_back(
        "infinite loop",
        "./test_data/scripts/bad/infinite.py",
        Request("GET", "/scripts/bad/infinite.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::GatewayTimeout, {}, ""));
    // pipe overflow - writes more than handler can buffer
    test_cases.emplace_back(
        "pipe overflow",
        "./test_data/scripts/bad/pipe_overflow.py",
        Request("GET", "/scripts/bad/pipe_overflow.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::GatewayTimeout, {}, "")); //@NOTE might be 502 alternatively
    // no output - empty response, no headers
    test_cases.emplace_back(
        "no output",
        "./test_data/scripts/bad/no_output.py",
        Request("GET", "/scripts/bad/no_output.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::BadGateway, {}, ""));
    // bad header format - missing colon
    test_cases.emplace_back(
        "bad header format",
        "./test_data/scripts/bad/bad_format_header.py",
        Request("GET", "/scripts/bad/bad_format_header.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::BadGateway, {}, ""));
    // wrong status code - valid format but invalid status
    test_cases.emplace_back(
        "wrong status code",
        "./test_data/scripts/bad/wrong_status_code.py",
        Request("GET", "/scripts/bad/wrong_status_code.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::BadGateway, {}, ""));
    // random output - unpredictable behavior
    test_cases.emplace_back(
        "random output",
        "./test_data/scripts/bad/random.py",
        Request("GET", "/scripts/bad/random.py", "", "HTTP/1.1", {}, "", {}, StatusCode::Ok),
        Response(StatusCode::BadGateway, {}, ""));
    // clang-format on
    return test_cases;
}

void test_good_CgiHandler(const TestCase& test)
{
    Logger::info("===============\nTest: '%s'", test.title.c_str());
    CgiHandler handler(test.request, *test.ctx, TEST_TIMEOUT);

    EventAction write_ev{EventAction::WantWrite, EventAction::ClientSocket, 3, test.conn.get()};
    EventAction read_ev{EventAction::WantRead, EventAction::ClientSocket, 3, test.conn.get()};
    EventAction* ev = &write_ev;

    while (!handler.done())
    {
        if (ev == &read_ev && handler.is_writing())
            ev = &write_ev;
        else
            ev = &read_ev;
        try
        {
            test.conn->set(*ev);
            handler.process();
            usleep(10000); // give initial time to os to setup the pipe/subprocess
        }
        catch (const ResponseError& error)
        {
            Logger::error("ResponseError: '%s'", error.msg().c_str());

            // these are not supposed to give an error
            std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
            return;
        }
    }

    Logger::debug_obj(handler.response(), "CgiHandler: response: ");

    if (handler.response().body_fd() != -1)
    {
        Logger::debug("test_CgiHandler: get the rest of the content from the pipe");

        // mock socket
        // default case: evalute status code and compare bodies
        // create tmp socket
        int sv[2];
        ::socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        ::fcntl(sv[0], F_SETFL, O_NONBLOCK);
        ::fcntl(sv[1], F_SETFL, O_NONBLOCK);

        // drain
        Response& res = const_cast<Response&>(handler.response());
        while (!res.done())
            res.send(sv[0]); // send to one end

        // read everything from the other end
        std::string received;
        char buffer[4096];
        ssize_t bytes;
        while ((bytes = ::read(sv[1], buffer, sizeof(buffer))) > 0)
            received.append(buffer, bytes);

        ::close(sv[0]);
        ::close(sv[1]);

        Logger::debug("test_CgiHandler: received:bytes=%lld,data='%s'", received.size(), received.c_str());

        // received contains status line + headers + body
        // strip status line and headers to get just the body
        size_t body_start = received.find(constants::crlfcrlf);
        std::string body = (body_start != std::string::npos) ? received.substr(body_start + 4) : "";

        if (body == test.expected.body())
            std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
        else
        {
            std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
            std::cerr << "body size got: " << body.size() << " expected: " << test.expected.body().size() << "\n";
        }
        return;
    }

    // compare status codes and bodies
    // @ASSUMPTIO body output of cgi is equal to request body - it just mirrors
    if (handler.response() == test.expected && tu::compare_bodies(handler.response(), test.expected))
    {
        std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
    }
    else
    {
        std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
        // std::cerr << "=====\nExpected:\n" << test.expected << "=====\nGot:\n" << handler.response();
        //  std::cerr << "(If the code is not different than there is a difference in the body)";
        //  std::cerr << "Expected: " << test.expected.body() << "\n";
        //  std::cerr << "Got: " << handler.response().body() << "\n";
    }
}

void test_bad_CgiHandler(const TestCase& test)
{
    Logger::info("Test: '%s'", test.title.c_str());
    CgiHandler handler(test.request, *test.ctx, TEST_TIMEOUT);

    EventAction write_ev{EventAction::WantWrite, EventAction::ClientSocket, 3, test.conn.get()};
    EventAction read_ev{EventAction::WantRead, EventAction::ClientSocket, 3, test.conn.get()};
    EventAction* ev = &write_ev;
    while (!handler.done())
    {
        if (ev == &read_ev && handler.is_writing())
            ev = &write_ev;
        else
            ev = &read_ev;
        try
        {
            test.conn->set(*ev);
            handler.process();
            usleep(10000); // give initial time to os to setup the pipe/subprocess
        }
        catch (const ResponseError& error)
        {
            Logger::error("ResponseError: '%s'", error.msg().c_str());

            if (error.status_code() == test.expected.status_code())
            {
                // these are supposed to give an error
                std::cerr << constants::green << "[OK] " << constants::reset << test.title << "\n";
            }
            else
            {
                // these are supposed to give an error
                std::cerr << constants::red << "[KO] " << constants::reset << test.title
                          << "\n\tExpected: " << test.expected.status_code() << "\n\tGot:      " << error.status_code()
                          << "\n";
            }
            return;
        }
    }

    // on error
    std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
    std::cerr << "=====\nExpected Error!\n"
              << "=====\nDidn't catch an error, Got:\n"
              << handler.response();
}

int main()
{
    Logger::set_global_level(Log::Error);

    // send output of cgi to here
    // to check what subprocess says
    // Logger::set_output("./logs/subprocess.log", std::ios::out | std::ios::trunc);

    std::cout << "==============================\n";
    std::cout << "========== CgiHandler ========\n";
    std::cout << "==============================\n";
    std::cout << "--> Testing with delay of " << TEST_TIMEOUT << " seconds!\n";

    std::vector<TestCase> tests;

    // good
    std::cout << constants::green << "\nGood tests\n" << constants::reset;
    tests = generate_good_test_cases();
    for (auto& test : tests)
    {
        // if (test.title != "custom status code")
        //     continue;
        test_good_CgiHandler(test);
    }

    // bad
    std::cout << constants::red << "\nBad tests\n" << constants::reset;
    tests = generate_bad_test_cases();
    int stop = 0;
    for (auto& test : tests)
    {
        if (stop == -1)
            break;
        test_bad_CgiHandler(test);
        ++stop;
    }

    Logger::flush();
    return 0;
}
