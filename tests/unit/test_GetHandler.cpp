#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config/types/LocationConfig.hpp"
#include "config/types/ServiceConfig.hpp"
#include "core/Logger.hpp"
#include "core/Path.hpp"
#include "core/constants.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "http/processor/handler/GetHandler.hpp"
#include "http/response/ResponseError.hpp"

#include "test_utils.hpp"

const std::string RESULT_FILE = "result.txt";
const std::string RESULT_BODY_FILE = "result_body.txt";
const std::string DIFF_FILE = "diff_log.txt";

struct TestCase
{
    TestCase(
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

    std::string title;
    Request request;
    Path file_path;
    LocationConfig location;
    ServiceConfig service;

    // Owned resources
    std::unique_ptr<Socket> socket;
    std::unique_ptr<EventManager> events;
    std::unique_ptr<RequestContext> ctx;

    Response expected;
};

std::vector<TestCase> generate_good_test_cases(void)
{
    // default service
    LocationConfig get{
        "/get",
        {{Token::DirectiveMethods, {"GET"}},
         {Token::DirectiveRoot, {"./test_data/get/"}},
         {Token::DirectiveDefaultFile, {"default.md"}}}};
    ServiceConfig service{get};

    std::vector<TestCase> test_cases = {};
    test_cases.reserve(100);

    // clang-format off

    // simple hello world
    test_cases.emplace_back(
        "simple hello",
        "./test_data/get/hello.md",
	service,
	get,
        Request("GET", "/get/hello.md", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );
    
    // Empty file
    test_cases.emplace_back(
        "empty file",
        "./test_data/get/empty.txt",
	service,
	get,
        Request("GET", "/get/empty.txt", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // Default file
    test_cases.emplace_back(
        "default file",
        "./test_data/get/default.md",
	service,
	get,
        Request("GET", "/get/", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // Large file (to test chunked reads)
    test_cases.emplace_back(
        "large file",
        "./test_data/get/large.txt",
	service,
	get,
        Request("GET", "/get/large.txt", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );
    
    // Different content types
    // html
    test_cases.emplace_back(
        "html file",
        "./test_data/get/index.html",
	service,
	get,
        Request("GET", "/get/index.html", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // jpeg
    test_cases.emplace_back(
        "jpeg file",
        "./test_data/get/giant_cat.jpeg",
	service,
	get,
        Request("GET", "/get/giant_cat.jpeg", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // json
    test_cases.emplace_back(
        "large json",
        "./test_data/get/large.json",
	service,
	get,
        Request("GET", "/get/large.json", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // xml
    test_cases.emplace_back(
        "xml",
        "./test_data/get/xml_file.xml",
	service,
	get,
        Request("GET", "/get/xml_file.xml", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // pdf
    test_cases.emplace_back(
        "434 page pdf",
        "./test_data/get/pdf_file.pdf",
	service,
	get,
        Request("GET", "/get/pdf_file.pdf", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // mp4
    test_cases.emplace_back(
        "5 sec video",
        "./test_data/get/test.mp4",
	service,
	get,
        Request("GET", "/get/test.mp4", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // -----------------------------------
    // Autoindex
    // set autoindex on
    Directive dir{Token::DirectiveListing, {"on"}};
    get.set(dir);

    test_cases.emplace_back(
        "autoindex",
        "./test_data/get/",
	service,
	get,
        Request("GET", "/get/", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    // -----------------------------------
    // Redirection
    // create redirection location/service
    LocationConfig redirect
    { "/redirect",
	    {
		{Token::DirectiveMethods, {"GET"}},
		{Token::DirectiveRedirect, {"301", "redirected_path.com"}}
	    }
    };
    ServiceConfig rsv{redirect};

    test_cases.emplace_back(
        "redirection",
        "./test_data/redirect/default.md",
	rsv,
	redirect,
        Request("GET", "/redirect/", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::MovedPermanently)
    );

    // clang-format on
    return test_cases;
}

std::vector<TestCase> generate_bad_test_cases(void)
{
    std::vector<TestCase> test_cases = {};
    test_cases.reserve(100);
    // clang-format off

    LocationConfig error{
        "/error",
        {{Token::DirectiveMethods, {"GET"}},
         {Token::DirectiveRoot, {"./test_data/get/error"}},
         }};
    ServiceConfig service{error};

    // File not found
    test_cases.emplace_back(
        "file not found",
        "./test_data/get/error/nonexistent.md",
        service,
        error,
        Request("GET", "/get/error/nonexistent.md", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::NotFound, {}, "")
    );

    // Directory 
    test_cases.emplace_back(
         "request is a directory",
         "./test_data/get/error/",
         service,
         error,
         Request("GET", "/get/error", "", "HTTP/1.1", {}, "", StatusCode::Ok),
         Response(StatusCode::Forbidden, {}, "")
     );

    // Nonexistent default file
     Directive directive{Token::DirectiveDefaultFile, {{"nonexistent.md"}}};
     error.set(directive);
     test_cases.emplace_back(
         "default file doesn't exist",
         "./test_data/get/error/",
         service,
         error,
         Request("GET", "/get/error", "", "HTTP/1.1", {}, "", StatusCode::Ok),
         Response(StatusCode::NotFound, {}, "")
     );

    // No permissions default file
    // @NOTE: chmod a-rwx no_perms.md
     directive = {Token::DirectiveDefaultFile, {{"no_perms.md"}}};
     error.set(directive);
     test_cases.emplace_back(
         "default has no permissions",
         "./test_data/get/error/",
         service,
         error,
         Request("GET", "/get/error", "", "HTTP/1.1", {}, "", StatusCode::Ok),
         Response(StatusCode::Forbidden, {}, "")
     );

    // No permissions requested file
     test_cases.emplace_back(
         "requested file has no permissions",
         "./test_data/get/error/no_perms.md",
         service,
         error,
         Request("GET", "/get/error/no_perms.md", "", "HTTP/1.1", {}, "", StatusCode::Ok),
         Response(StatusCode::Forbidden, {}, "")
     );

    // clang-format on
    return test_cases;
}

void test_good_GetHandler(const TestCase& test)
{
    Logger::info("===============\nTest: '%s'", test.title.c_str());
    GetHandler gh(test.request, *test.ctx);

    // process request
    while (!gh.done())
    {
        try
        {
            gh.process();
        }
        catch (const ResponseError& error)
        {
            Logger::error("ResponseError: '%s'", error.msg().c_str());
            // these are not supposed to give an error
            std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
            return;
        }
    }

    // get response
    Response res = gh.response();
    Logger::debug_obj(res, "GetHandler: response: ");

    // handle specific cases
    if (test.title == "redirection" || test.title == "autoindex")
    {
        if (res.status_code() == test.expected.status_code())
            std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
        else
            std::cout << constants::red << "[KO]! " << constants::reset << test.title
                      << "\n\tExpected code: " << test.expected.status_code() << "\n\tGot code: " << res.status_code()
                      << "\n";
        return;
    }

    // default case: evalute status code and compare bodies
    // create tmp socket
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    fcntl(sv[0], F_SETFL, O_NONBLOCK);
    fcntl(sv[1], F_SETFL, O_NONBLOCK);

    // socket -> buffer -> file
    char buf[constants::read_chunk_size + 1];
    std::ofstream out(RESULT_FILE);
    ssize_t n;
    while (!res.done())
    {
        res.send(sv[0]);
        while ((n = read(sv[1], buf, constants::read_chunk_size)) > 0)
        {
            out.write(buf, n);
        }
    }
    close(sv[0]);
    close(sv[1]);
    out.flush();

    // create file only  based off response body
    std::string result_str = utils::file_to_str(RESULT_FILE.c_str());
    size_t pos = result_str.find(constants::crlfcrlf) + 4;
    std::string body_str = result_str.substr(pos);
    std::ofstream body_file(RESULT_BODY_FILE);
    body_file << body_str;
    body_file.flush();

    // diff expected vs result
    int diff_result = test_utils::diff_and_log(RESULT_BODY_FILE.c_str(), test.file_path.raw.c_str(), DIFF_FILE.c_str());
    if (diff_result == 0 && test.expected.status_code() == res.status_code())
    {
        std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
    }
    else
    {
        std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
        // clang-format off
        std::cerr << "=====\nExpected:\n"
		  << "Status: " << test.expected.status_code() << "\n"
                  << test.file_path.raw.c_str() << "':\n'" << utils::file_to_str(test.file_path.raw.c_str()) << "'\n"
                  << "=====\nGot:\n"
		  << "Status: " << res.status_code() << "\n"
                  << RESULT_BODY_FILE << "':\n'" << utils::file_to_str(RESULT_BODY_FILE.c_str()) << "'\n"
		  << "=====\nDiff:\n'"
		  << DIFF_FILE << "':\n'" << utils::file_to_str(DIFF_FILE.c_str()) << "'\n";
        // clang-format on
    }
}

void test_bad_GetHandler(const TestCase& test)
{
    Logger::info("===============\nTest: '%s'", test.title.c_str());
    GetHandler gh(test.request, *test.ctx);

    // process request
    while (!gh.done())
    {
        try
        {
            gh.process();
        }
        catch (const ResponseError& error)
        {
            Logger::error("ResponseError: '%s'", error.msg().c_str());
            Logger::debug_obj(error, "GetHandler: error response: ");
            // should error
            if (error.status_code() == test.expected.status_code())
                std::cerr << constants::green << "[OK] " << constants::reset << test.title << "\n";
            else
            {
                std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
                std::cerr << "=====\nExpected:\n"
                          << "Status: " << test.expected.status_code() << "\n"
                          << "=====\nGot:\n"
                          << "Status: " << error.status_code() << "\n";
            }
            return;
        }
    }

    // get response
    Response res = gh.response();
    Logger::debug_obj(res, "GetHandler: response: ");

    // default case: evalute status code and compare bodies
    // create tmp socket
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    fcntl(sv[0], F_SETFL, O_NONBLOCK);
    fcntl(sv[1], F_SETFL, O_NONBLOCK);

    // socket -> buffer -> file
    char buf[constants::read_chunk_size + 1];
    std::ofstream out(RESULT_FILE);
    ssize_t n;
    while (!res.done())
    {
        res.send(sv[0]);
        while ((n = read(sv[1], buf, constants::read_chunk_size)) > 0)
        {
            out.write(buf, n);
        }
    }
    close(sv[0]);
    close(sv[1]);
    out.flush();

    std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
    // clang-format off
    std::cerr << "=====\nExpected:\n"
	  << "Status: " << test.expected.status_code() << "\n"
          << "=====\nGot:\n"
	  << "Status: " << res.status_code() << "\n";
    // clang-format on
}

int main(void)
{
    Logger::set_global_level(Log::Fatal);

    Logger::trace("=================");
    Logger::info("GET HANDLER START");

    std::cout << "\n===== GET tests====\n";

    std::cout << "\nGood\n";

    std::vector<TestCase> tests = generate_good_test_cases();
    for (auto& test : tests)
    {

        try
        {
            test_good_GetHandler(test);
        }
        catch (const std::exception& e)
        {
            Logger::fatal("Test: '%s'", e.what());
        }
    }

    std::cout << "\nBad\n";

    tests = generate_bad_test_cases();

    for (auto& test : tests)
    {
        try
        {
            test_bad_GetHandler(test);
        }
        catch (const std::exception& e)
        {
            Logger::fatal("Test: '%s'", e.what());
        }
    }

    Logger::flush();
    return 0;
}
