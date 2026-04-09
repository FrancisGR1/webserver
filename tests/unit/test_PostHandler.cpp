#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "http/processor/handler/PostHandler.hpp"
#include "http/response/ResponseError.hpp"

#include "test_utils.hpp"

static constexpr std::string_view DIFF_FILE = "./test_data/logs/ph_diff_log.txt";
static const std::string TEST_FILES = "./test_data/post/send/";

// Objective is to test main PostHandler pathways, which are:
// - Integrity of the uploaded file
// - Multiple types of files
// - Wrong configurations
std::vector<tu::HandlerTestCase> generate_good_test_cases(void)
{
    // clang-format off
    std::vector<tu::HandlerTestCase> test_cases;
    test_cases.reserve(100);

    // to upload huge files
    auto max = std::to_string(std::numeric_limits<std::size_t>::max());

    // default service
    LocationConfig post{
        "/upload",
        {{Token::DirectiveMethods, {"POST"}},
         {Token::DirectiveUpload, {"on"}},
         {Token::DirectiveRoot, {"./test_data/post/upload"}},
         {Token::DirectiveMaxBodySize, {max.c_str()}}}
    };

    ServiceConfig service{post};

    //@NOTE: instead of putting the str in the request/response body we put the file
    // path which contains the content corresponding to the body.
    // This makes it easier to insert/validate data

    const std::string hello_md = TEST_FILES + "hello.md";
    test_cases.emplace_back(
        "simple hello",
        "./test_data/post/upload/",
        service,
        post,
        Request( "POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), StatusCode::Ok), // @NOTE: insert content of hello_md in request, @TODO: this is why we can only make small tests! Change this.
        Response( StatusCode::Created, {}, hello_md) //@NOTE: response body contains file path to expected body
	);

    const std::string small_jpg = TEST_FILES + "small.jpg";
    test_cases.emplace_back(
        "binary png - integrity check",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(small_jpg), StatusCode::Ok),
        Response(StatusCode::Created, {}, small_jpg));

    const std::string empty_txt = TEST_FILES + "empty.txt";
    test_cases.emplace_back(
        "empty file",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(empty_txt), StatusCode::Ok),
        Response(StatusCode::Created, {}, empty_txt));

    const std::string utf8_txt  = TEST_FILES + "utf8.txt";
    test_cases.emplace_back(
        "utf8 content",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(utf8_txt), StatusCode::Ok),
        Response(StatusCode::Created, {}, utf8_txt));

    const std::string large_txt = TEST_FILES + "large.txt";
    test_cases.emplace_back(
        "large text file",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(large_txt), StatusCode::Ok),
        Response(StatusCode::Created, {}, large_txt));

    const std::string pdf_file_pdf = TEST_FILES + "pdf_file.pdf";
    test_cases.emplace_back(
        "pdf file",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(pdf_file_pdf), StatusCode::Ok),
        Response(StatusCode::Created, {}, pdf_file_pdf));

    // --- overwrite: upload same file twice, result must still match ---
    test_cases.emplace_back(
        "overwrite same file",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), StatusCode::Ok),
        Response(StatusCode::Created, {}, hello_md));

    // --- different root ---
    LocationConfig post_alt_root{
        "/upload",
        {{Token::DirectiveMethods, {"POST"}},
         {Token::DirectiveUpload, {"on"}},
         {Token::DirectiveRoot, {"./test_data/post/upload_alt"}}}};
    ServiceConfig service_alt_root{post_alt_root};

    test_cases.emplace_back(
        "different root dir",
        "./test_data/post/upload_alt/",
        service_alt_root, post_alt_root,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), StatusCode::Ok),
        Response(StatusCode::Created, {}, hello_md));

    // clang-format on

    return test_cases;
}

std::string get_expected_path(std::string path)
{
    std::string result(path);
    size_t pos = result.find("/send/");
    if (pos != std::string::npos)
        result.replace(pos, 6, "/expected/");
    return result;
}

// tester functions
// compares status code in case of error
// compares status code and content in case of no error
void test_good_PostHandler(const tu::HandlerTestCase& test)
{
    // upload response
    // compare files
    Logger::info("===============\nTest: '%s'", test.title.c_str());
    PostHandler handler{test.request, *test.ctx};
    Logger::debug_obj(*test.ctx->config().location(), "Config:\n");
    Logger::debug_obj(test.request, "Request:\n");
    // process request
    while (!handler.done())
    {
        try
        {
            handler.process();
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
    Response res = handler.response();
    Logger::debug_obj(res, "PostHandler: response: ");

    // handle specific cases
    if (test.title == "redirection")
    {
        if (res.status_code() == test.expected.status_code())
            std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
        else
            std::cout << constants::red << "[KO]! " << constants::reset << test.title
                      << "\n\tExpected code: " << test.expected.status_code() << "\n\tGot code: " << res.status_code()
                      << "\n";
        return;
    }

    // diff expected vs result
    std::string expected_path = get_expected_path(test.expected.body());
    std::string test_path = res.headers().at("Location"); //@ASSUMPTION: response has a "Location"
    int diff_result = tu::diff_and_log(expected_path.c_str(), test_path.c_str(), DIFF_FILE.data());
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
                  << test.expected.body().c_str() << "':\n'" << utils::file_to_str(test.expected.body().c_str()) << "'\n"
                  << "=====\nGot:\n"
		  << "Status: " << res.status_code() << "\n"
                  << expected_path << "':\n'" << utils::file_to_str(expected_path.c_str()) << "'\n"
		  << "=====\nDiff:\n'"
		  << DIFF_FILE << "':\n'" << utils::file_to_str(DIFF_FILE.data()) << "'\n";
        // clang-format on
    }
}

int main()
{
    Logger::set_global_level(Log::Warn);

    Logger::trace("=================");
    Logger::info("POST HANDLER START");

    std::cout << "\n===== GET tests====\n";

    std::cout << "\nGood\n";

    std::vector<tu::HandlerTestCase> tests = generate_good_test_cases();
    int idx = 0;
    for (auto& test : tests)
    {
        try
        {
            test_good_PostHandler(test);
        }
        catch (const std::exception& e)
        {
            Logger::fatal("Test: '%s'", e.what());
        }
        if (idx == -1) // @NOTE: use this to test until a certain point
            break;
        ++idx;
    }
}
