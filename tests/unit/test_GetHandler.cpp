#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config/types/Directive.hpp"
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
    TestCase(const std::string title, Path file_path, Request req, Response expected)
        : title(title)
        , request(req)
        , file_path(file_path)
        , expected(expected)
    {
        auto directive = Directive{Token::DirectiveMethods, {"GET"}};
        file_location.set(directive);
        service = ServiceConfig{{file_location}};
        socket = std::make_unique<Socket>(3, service); // 3 = dummy fd
        events = std::make_unique<EventManager>();
        ctx = std::make_unique<RequestContext>(*socket, *events, service);

        ctx->config().set(file_path);
        ctx->config().set(file_location);

        //@TODO adicionar response defaults
    }

    std::string title;
    Request request;
    Path file_path;
    ServiceConfig service;
    LocationConfig file_location;

    // Owned resources
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
        "simple hello",
        "./test_data/get/hello.md",
        Request("GET", "/get/hello.md", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );
    
    // Empty file
    test_cases.emplace_back(
        "empty file",
        "./test_data/get/empty.txt",
        Request("GET", "/get/empty.txt", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );
    
    // Large file (to test chunked reads)
    test_cases.emplace_back(
        "large file",
        "./test_data/get/large.txt",
        Request("GET", "/get/large.txt", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );
    
    // Different content types
    test_cases.emplace_back(
        "html file",
        "./test_data/get/index.html",
        Request("GET", "/get/index.html", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    test_cases.emplace_back(
        "jpeg file",
        "./test_data/get/giant_cat.jpeg",
        Request("GET", "/get/giant_cat.jpeg", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    test_cases.emplace_back(
        "large json",
        "./test_data/get/large.json",
        Request("GET", "/get/large.json", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    test_cases.emplace_back(
        "xml",
        "./test_data/get/xml_file.xml",
        Request("GET", "/get/xml_file.xml", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    test_cases.emplace_back(
        "434 page pdf",
        "./test_data/get/pdf_file.pdf",
        Request("GET", "/get/pdf_file.pdf", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );

    test_cases.emplace_back(
        "5 sec video",
        "./test_data/get/test.mp4",
        Request("GET", "/get/test.mp4", "", "HTTP/1.1", {}, "", StatusCode::Ok),
        Response(StatusCode::Ok)
    );
    //@TODO adicionar imagem, vídeo, etc?

    // clang-format on
    return test_cases;
}

std::vector<TestCase> generate_bad_test_cases(void)
{
    std::vector<TestCase> test_cases = {};
    test_cases.reserve(100);
    // clang-format off

    // File not found
    test_cases.emplace_back(
        "file not found",
        "./test_data/get/nonexistent.md",
        Request("GET", "/get/nonexistent.md", "", "HTTP/1.1", {}, "", StatusCode::NotFound),
        Response(StatusCode::NotFound, {}, "")
    );
    
    // Directory (should return 403 or 301 depending on your server)
    test_cases.emplace_back(
        "request is a directory",
        "./test_data/get/",
        Request("GET", "/get/", "", "HTTP/1.1", {}, "", StatusCode::Forbidden),
        Response(StatusCode::Forbidden, {}, "")
    );
    
    // Path traversal attempt (should be blocked)
    test_cases.emplace_back(
        "path traversal",
        "./test_data/get/../../../etc/passwd",
        Request("GET", "/../../../etc/passwd", "", "HTTP/1.1", {}, "", StatusCode::Forbidden),
        Response(StatusCode::Forbidden, {}, "")
    );
    // clang-format on
    return test_cases;
}

void test_GetHandler(const TestCase& test)
{
    Logger::info("===============\nTest: '%s'", test.title.c_str());
    GetHandler gh(test.request, *test.ctx);

    while (!gh.done())
    {
        try
        {
            gh.process();
        }
        catch (const ResponseError& error)
        {
            ;
            Logger::error("ResponseError: '%s'", error.msg().c_str());

            // these are not supposed to give an error
            std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
            return;
        }
    }

    Response res = gh.response();
    Logger::debug_obj(res, "GetHandler: response: ");
    // create tmp socket
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    fcntl(sv[0], F_SETFL, O_NONBLOCK);
    fcntl(sv[1], F_SETFL, O_NONBLOCK);

    // read result out of sv[1] into a file
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

    int diff_result = test_utils::diff_and_log(RESULT_BODY_FILE.c_str(), test.file_path.raw.c_str(), DIFF_FILE.c_str());
    if (diff_result == 0 && test.expected.status_code() == res.status_code())
    {
        std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
    }
    else
    {
        std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
        // clang-format off
        std::cerr << "=====\nExpected:\n'"
		  << "Status: " << test.expected.status_code() << "\n"
                  << test.file_path.raw.c_str() << "':\n'" << utils::file_to_str(test.file_path.raw.c_str()) << "'\n"
                  << "=====\nGot:\n'"
		  << "Status: " << res.status_code() << "\n"
                  << RESULT_BODY_FILE << "':\n'" << utils::file_to_str(RESULT_BODY_FILE.c_str()) << "'\n"
		  << "=====\nDiff:\n'"
		  << DIFF_FILE << "':\n'" << utils::file_to_str(DIFF_FILE.c_str()) << "'\n";
        // clang-format on
    }
}

int main(void)
{
    Logger::set_global_level(Log::Warn);

    Logger::trace("=================");
    Logger::info("GET HANDLER START");

    std::cout << "\n=====GET tests====\n";

    std::vector<TestCase> tests = generate_good_test_cases();

    for (auto& test : tests)
    {
        try
        {
            test_GetHandler(test);
        }
        catch (const std::exception& e)
        {
            Logger::fatal("Test: '%s'", e.what());
        }
    }

    Logger::flush();
    return 0;
}
