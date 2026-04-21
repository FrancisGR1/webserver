#include <cstdlib>
#include <cstring>
#include <fstream>
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
static const std::string POST_DIR = "./test_data/post";

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
        Request( "POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), {}, StatusCode::Ok), // @NOTE: insert content of hello_md in request, @TODO: this is why we can only make small tests! Change this.
        Response( StatusCode::Created, {}, hello_md) //@NOTE: response body contains file path to expected body
	);

    const std::string small_jpg = TEST_FILES + "small.jpg";
    test_cases.emplace_back(
        "binary png - integrity check",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(small_jpg), {}, StatusCode::Ok),
        Response(StatusCode::Created, {}, small_jpg));

    const std::string empty_txt = TEST_FILES + "empty.txt";
    test_cases.emplace_back(
        "empty file",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(empty_txt), {}, StatusCode::Ok),
        Response(StatusCode::Created, {}, empty_txt));

    const std::string utf8_txt  = TEST_FILES + "utf8.txt";
    test_cases.emplace_back(
        "utf8 content",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(utf8_txt), {}, StatusCode::Ok),
        Response(StatusCode::Created, {}, utf8_txt));

    const std::string large_txt = TEST_FILES + "large.txt";
    test_cases.emplace_back(
        "large text file",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(large_txt), {}, StatusCode::Ok),
        Response(StatusCode::Created, {}, large_txt));

    const std::string pdf_file_pdf = TEST_FILES + "pdf_file.pdf";
    test_cases.emplace_back(
        "pdf file",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(pdf_file_pdf), {}, StatusCode::Ok),
        Response(StatusCode::Created, {}, pdf_file_pdf));

    // --- overwrite: upload same file twice, result must still match ---
    test_cases.emplace_back(
        "overwrite same file",
        "./test_data/post/upload/",
        service, post,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), {}, StatusCode::Ok),
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
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), {}, StatusCode::Ok),
        Response(StatusCode::Created, {}, hello_md));

    // clang-format on

    return test_cases;
}

std::vector<tu::HandlerTestCase> generate_bad_test_cases(void)
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

    // will be used as a dummy file -> should never have to compare uploads
    const std::string hello_md = TEST_FILES + "hello.md";

    //@NOTE: instead of putting the str in the request/response body we put the file
    // path which contains the content corresponding to the body.
    // This makes it easier to insert/validate data

    // --- post not allowed ---
    LocationConfig no_post{
        "/upload",
        {{Token::DirectiveMethods, {"GET"}},
         {Token::DirectiveUpload, {"on"}},
         {Token::DirectiveRoot, {"./test_data/post/upload"}},
         {Token::DirectiveMaxBodySize, {max.c_str()}}}
    };
    ServiceConfig service_no_post{no_post};
    test_cases.emplace_back(
        "no post allowed",
        "./test_data/post/upload/",
        service_no_post,
        no_post,
        Request( "POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), {}, StatusCode::Ok), // @NOTE: insert content of hello_md in request, @TODO: this is why we can only make small tests! Change this.
        Response( StatusCode::MethodNotAllowed, {}, hello_md ) //@NOTE: response body contains file path to expected body //@QUESTION: que erro é que o deve ser dado? neste momento o server está a dar InternalServerError
	);

    // --- disable upload ---
    LocationConfig no_upload_allowed = post;
    Directive off = {Token::DirectiveUpload, {"off"}};
    no_upload_allowed.set(off);
    ServiceConfig no_upload{no_upload_allowed};

    test_cases.emplace_back(
        "disabled upload",
        "./test_data/post/upload/",
        no_upload,
        no_upload_allowed,
        Request( "POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), {}, StatusCode::Ok), // @NOTE: insert content of hello_md in request, @TODO: this is why we can only make small tests! Change this.
        Response( StatusCode::Forbidden, {}, hello_md ) //@NOTE: response body contains file path to expected body //@QUESTION: que erro é que o deve ser dado? neste momento o server está a dar InternalServerError
	);

    // --- forbidden ---
    // @NOTE: do a chmod 000 on the directory
    
    const std::string no_perm_dir = "./test_data/post/forbidden";
    mkdir(no_perm_dir.c_str(), 0777); 
    LocationConfig forbidden{
        "/forbidden",
        {{Token::DirectiveMethods, {"POST"}},
         {Token::DirectiveUpload, {"on"}},
         {Token::DirectiveRoot, {no_perm_dir.c_str()}},
         {Token::DirectiveMaxBodySize, {max.c_str()}}}
    };
    ServiceConfig forbidden_service{forbidden};
    // remove write and execute perms
    chmod(no_perm_dir.c_str(), 0555); 

    test_cases.emplace_back(
        "forbidden",
        "./test_data/post/forbidden/",
        forbidden_service,
        forbidden,
        Request( "POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), {}, StatusCode::Ok), // @NOTE: insert content of hello_md in request, @TODO: this is why we can only make small tests! Change this.
        Response( StatusCode::Forbidden, {}, hello_md ) //@NOTE: response body contains file path to expected body //@QUESTION: que erro é que o deve ser dado? neste momento o server está a dar InternalServerError
	);

    // --- body too large ---
    LocationConfig no_content{
        "/no_content",
        {{Token::DirectiveMethods, {"POST"}},
         {Token::DirectiveUpload, {"on"}},
         {Token::DirectiveRoot, {"./test_data/post/"}},
         {Token::DirectiveMaxBodySize, {"1"}}}
    };
    ServiceConfig no_content_service{no_content};
    test_cases.emplace_back(
        "body exceeds max body size",
        "./test_data/post/upload/",
        no_content_service,
        no_content,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, "this body is way too long", {}, StatusCode::Ok),
        Response(StatusCode::ContentTooLarge, {}, hello_md)
    );

    // --- upload directory does not exist ---
    LocationConfig nonexistent_dir{
        "/upload",
        {{Token::DirectiveMethods, {"POST"}},
         {Token::DirectiveUpload, {"on"}},
         {Token::DirectiveRoot, {"./test_data/post/nonexistent_dir"}},
         {Token::DirectiveMaxBodySize, {max.c_str()}}}
    };
    ServiceConfig nonexistent_dir_service{nonexistent_dir};
    test_cases.emplace_back(
        "upload directory does not exist",
        "./test_data/post/nonexistent_dir/",
        nonexistent_dir_service,
        nonexistent_dir,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), {}, StatusCode::Ok),
        Response(StatusCode::NotFound, {}, hello_md)
    );

    // --- upload directory is a file, not a directory ---
    // create
    const std::string file_not_dir = std::string("./test_data/get/error/") + "no_perms.md";
    std::ofstream(file_not_dir.c_str()).close();

    LocationConfig file_as_dir{
        "/upload",
        {{Token::DirectiveMethods, {"POST"}},
         {Token::DirectiveUpload, {"on"}},
         {Token::DirectiveRoot, {file_not_dir.c_str()}},
         {Token::DirectiveMaxBodySize, {max.c_str()}}}
    };
    ServiceConfig file_as_dir_service{file_as_dir};
    test_cases.emplace_back(
        "upload root is a file not a directory",
        "./test_data/post/upload/hello.md",
        file_as_dir_service,
        file_as_dir,
        Request("POST", "/upload/", "", "HTTP/1.1", {}, utils::file_to_str(hello_md), {}, StatusCode::Ok),
        Response(StatusCode::Conflict, {}, hello_md)
    );

    //@TODO testar redirection uri

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

    // post response
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
    std::string test_path = handler.upload_path().raw;
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

// upload response
// compare files
void test_bad_PostHandler(const tu::HandlerTestCase& test)
{
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
            Logger::trace("ResponseError: '%s'", error.msg().c_str());
            // these are supposed to give an error
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

    // post response
    Response res = handler.response();
    Logger::debug_obj(res, "PostHandler: response: ");

    // diff expected vs result
    std::string expected_path = get_expected_path(test.expected.body());
    std::string test_path = handler.upload_path().raw;

    // log
    std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
    std::cerr << "-> Didn't give an error!\n";
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

int main()
{
    Logger::set_global_level(Log::Warn);

    std::cout << "==============================\n";
    std::cout << "========= PostHandler ========\n";
    std::cout << "==============================\n";

    // good
    std::cout << constants::green << "\nGood tests\n" << constants::reset;
    std::vector<tu::HandlerTestCase> tests = generate_good_test_cases();
    int stop = 0;
    for (auto& test : tests)
    {
        if (stop == -1) // @NOTE: use this to test until a certain point
            break;
        try
        {
            test_good_PostHandler(test);
        }
        catch (const std::exception& e)
        {
            Logger::fatal("Test: '%s'", e.what());
        }
        ++stop;
    }

    // bad
    std::cout << constants::red << "\nBad tests\n" << constants::reset;
    tests = generate_bad_test_cases();
    stop = 0;
    for (auto& test : tests)
    {
        if (stop == -1) // @NOTE: use this to test until a certain point
            break;
        try
        {
            test_bad_PostHandler(test);
        }
        catch (const std::exception& e)
        {
            Logger::fatal("Test: '%s'", e.what());
        }
        ++stop;
    }
}
