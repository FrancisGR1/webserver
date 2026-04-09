#include <iostream>
#include <string>
#include <vector>

#include "core/Logger.hpp"
#include "http/StatusCode.hpp"
#include "http/request/Request.hpp"
#include "http/request/RequestParser.hpp"

struct TestCase
{
    std::string title;
    std::string raw_request;
    Request expected;
};

void test_RequestParser(const TestCase& test);
void test_RequestParser_with_random_sized_chunks(const TestCase& test);

std::vector<TestCase> generate_good_test_cases(void)
{
    //@NOTE: headers have to be lowercase because of normalization
    std::vector<TestCase> test_cases = {
        // Basic Methods
        {"Simple GET request",
         "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("GET", "/index.html", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},
        {"Simple POST with body",
         "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 11\r\n\r\nhello world",
         Request(
             "POST",
             "/submit",
             "",
             "HTTP/1.1",
             {{"host", "localhost"}, {"content-length", "11"}},
             "hello world",
             StatusCode::Ok)},
        {"DELETE request",
         "DELETE /resource/42 HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("DELETE", "/resource/42", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},
        // ─── Query Strings ───────────────────────────────────────────────
        {"GET with query string",
         "GET /search?q=hello HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("GET", "/search", "q=hello", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},
        {"GET with multi-param query string",
         "GET /search?q=hello&page=2&sort=asc HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("GET", "/search", "q=hello&page=2&sort=asc", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},
        {"GET with empty query string",
         "GET /index.html? HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("GET", "/index.html", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},

        // ─── Headers ─────────────────────────────────────────────────────
        {"Multiple headers",
         "GET / HTTP/1.1\r\nHost: localhost\r\nAccept: text/html\r\nConnection: keep-alive\r\n\r\n",
         Request(
             "GET",
             "/",
             "",
             "HTTP/1.1",
             {{"host", "localhost"}, {"accept", "text/html"}, {"connection", "keep-alive"}},
             "",
             StatusCode::Ok)},
        {"Header with leading whitespace in value",
         "GET / HTTP/1.1\r\nHost:   localhost\r\n\r\n",
         Request("GET", "/", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},

        // ─── Paths ───────────────────────────────────────────────────────
        {"Root path",
         "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("GET", "/", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},
        {"Nested path",
         "GET /a/b/c/d HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("GET", "/a/b/c/d", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},
        {"Path with file extension",
         "GET /static/style.css HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("GET", "/static/style.css", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},
        // ─── HTTP Versions ───────────────────────────────────────────────
        {"HTTP/1.0 request",
         "GET /index.html HTTP/1.0\r\nHost: localhost\r\n\r\n",
         Request("GET", "/index.html", "", "HTTP/1.0", {{"host", "localhost"}}, "", StatusCode::Ok)},

        // ─── POST edge cases ─────────────────────────────────────────────
        {"POST with Content-Length 0",
         "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n",
         Request(
             "POST", "/submit", "", "HTTP/1.1", {{"host", "localhost"}, {"content-length", "0"}}, "", StatusCode::Ok)},

        // ─── Headers edge cases ──────────────────────────────────────────
        {"Header value with internal whitespace",
         "GET / HTTP/1.1\r\nHost: localhost\r\nAccept: text/html, application/json\r\n\r\n",
         Request(
             "GET",
             "/",
             "",
             "HTTP/1.1",
             {{"host", "localhost"}, {"accept", "text/html, application/json"}},
             "",
             StatusCode::Ok)},
        {"Header with tab as OWS",
         "GET / HTTP/1.1\r\nHost:\tlocalhost\r\n\r\n",
         Request("GET", "/", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},

        // ─── Chunked body ────────────────────────────────────────────────
        {"Chunked transfer encoding single chunk",
         "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n",
         Request(
             "POST",
             "/upload",
             "",
             "HTTP/1.1",
             {{"host", "localhost"}, {"transfer-encoding", "chunked"}},
             "hello",
             StatusCode::Ok)},
        {"Chunked transfer encoding multiple chunks",
         "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n "
         "world\r\n0\r\n\r\n",
         Request(
             "POST",
             "/upload",
             "",
             "HTTP/1.1",
             {{"host", "localhost"}, {"transfer-encoding", "chunked"}},
             "hello world",
             StatusCode::Ok)},
        {"Chunked with chunk extension",
         "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: "
         "chunked\r\n\r\n5;ext=val\r\nhello\r\n0\r\n\r\n",
         Request(
             "POST",
             "/upload",
             "",
             "HTTP/1.1",
             {{"host", "localhost"}, {"transfer-encoding", "chunked"}},
             "hello",
             StatusCode::Ok)},
        {"Chunked with trailer header",
         "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\nX-Checksum: "
         "abc123\r\n\r\n",
         Request(
             "POST",
             "/upload",
             "",
             "HTTP/1.1",
             {{"host", "localhost"}, {"transfer-encoding", "chunked"}, {"x-checksum", "abc123"}},
             "hello",
             StatusCode::Ok)},

        // ─── Path edge cases ─────────────────────────────────────────────
        {"Path with percent-encoding",
         "GET /path%20with%20spaces HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("GET", "/path%20with%20spaces", "", "HTTP/1.1", {{"host", "localhost"}}, "", StatusCode::Ok)},

    };
    return test_cases;
}

std::vector<TestCase> generate_bad_test_cases(void)
{
    std::vector<TestCase> test_cases = {
        // ─── Bad Requests ─────────────────────────────────────────────────
        {"Missing HTTP version",
         "GET /index.html\r\nHost: localhost\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Wrong HTTP version",
         "GET /index.html HTTP/2\r\nHost: localhost\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::HttpVersionNotSupported)},
        {"Empty request", "", Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Invalid method",
         "FOOBAR / HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Unsupported HTTP version",
         "GET / HTTP/2.0\r\nHost: localhost\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Missing CRLF terminator",
         "GET / HTTP/1.1\r\nHost: localhost\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"PUT request with body",
         "PUT /resource/42 HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4\r\n\r\ndata",
         Request(
             "PUT",
             "/resource/42",
             "",
             "HTTP/1.1",
             {{"host", "localhost"}, {"content-length", "4"}},
             "data",
             StatusCode::NotImplemented)},
        // ─── Body errors ─────────────────────────────────────────────────
        {"Content-Length and Transfer-Encoding together",
         "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\nhello",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Negative Content-Length",
         "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: -1\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Non-digit Content-Length",
         "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: abc\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Content-Length exceeds max body size",
         "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 99999999999\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::ContentTooLarge)},
        {"Transfer-Encoding not chunked",
         "POST /submit HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: gzip\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::NotImplemented)},

        // ─── Header errors ───────────────────────────────────────────────
        {"Duplicate headers",
         "GET / HTTP/1.1\r\nHost: localhost\r\nHost: other\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Header key with invalid char",
         "GET / HTTP/1.1\r\nHo@st: localhost\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Header value with control character",
         "GET / HTTP/1.1\r\nHost: local\x01host\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},

        // ─── Path errors ─────────────────────────────────────────────────
        {"Path not starting with slash",
         "GET relative/path HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},
        {"Whitespace before method",
         " GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)},

        // ─── Line ending errors ──────────────────────────────────────────
        {"LF only line endings instead of CRLF",
         "GET / HTTP/1.1\nHost: localhost\n\n",
         Request("", "", "", "", {}, "", StatusCode::BadRequest)}

    };
    return test_cases;
}

int main()
{
    Logger::set_global_level(Log::Fatal);

    std::cout << "\nNote: one test: \n[Full]\n[Chunked]\n";

    std::vector<TestCase> tests;

    // good
    std::cout << constants::green << "\nGood tests\n" << constants::reset;
    tests = generate_good_test_cases();
    for (auto& test : tests)
    {
        test_RequestParser(test);
        test_RequestParser_with_random_sized_chunks(test);
    }

    // bad
    std::cout << constants::red << "\nBad tests\n" << constants::reset;
    tests = generate_bad_test_cases();
    for (auto& test : tests)
    {
        test_RequestParser(test);
        test_RequestParser_with_random_sized_chunks(test);
    }
    return 0;
}

void test_RequestParser(const TestCase& test)
{
    RequestParser parser;

    parser.feed(test.raw_request);
    Request result = parser.get();

    // check
    // if status is an error, only compare the code
    if (test.expected.status_code() >= StatusCode::BadRequest)
    {
        if (result.status_code() == test.expected.status_code())
        {
            std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
        }
        else
        {
            std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
            std::cerr << "----------\nGot Status Code: " << result.status_code()
                      << "\n----------\nExpected Status Code: " << test.expected.status_code() << "\n---------\n";
            std::cerr << "\n\n";
        }
    }
    else
    {
        if (result == test.expected)
        {
            std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
        }
        else
        {
            std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
            std::cerr << "----------\nGot:\n"
                      << result << "Status Code: " << test.expected.status_code() << "\n----------\nExpected:\n"
                      << test.expected << "Status Code: " << test.expected.status_code() << "\n---------\n";
            std::cerr << "\n\n";
        }
    }
}

void test_RequestParser_with_random_sized_chunks(const TestCase& test)
{
    RequestParser parser;

    int at = 0;
    while (!parser.done() && at < static_cast<int>(test.raw_request.size()))
    {
        // generate random number for the size of the request
        int random = 5; // rand() % test.raw_request.size();

        // now use it to make a substr
        std::string chunk = test.raw_request.substr(at, random);

        // feed the substr
        parser.feed(chunk);

        // advance
        at += random;
    }

    Request result = parser.get();

    // check
    // if status is an error, only compare the code
    if (test.expected.status_code() >= StatusCode::BadRequest)
    {
        if (result.status_code() == test.expected.status_code())
        {
            std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
        }
        else
        {
            std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
            std::cerr << "----------\nGot Status Code: " << result.status_code()
                      << "\n----------\nExpected Status Code: " << test.expected.status_code() << "\n---------\n";
            std::cerr << "\n\n";
        }
    }
    else
    {
        if (result == test.expected)
        {
            std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
        }
        else
        {
            std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
            std::cerr << "----------\nGot:\n"
                      << result << "Status Code: " << test.expected.status_code() << "\n----------\nExpected:\n"
                      << test.expected << "Status Code: " << test.expected.status_code() << "\n---------\n";
            std::cerr << "\n\n";
        }
    }
}
