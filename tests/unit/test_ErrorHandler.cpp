#include <fstream>
#include <sstream>
#include <string>

#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "http/processor/handler/ErrorHandler.hpp"
#include "http/response/ResponseError.hpp"

#include "test_utils.hpp"

static const std::string ERROR_FILES = "./test_data/error/";

struct ErrorHandlerTestCase
{
    ErrorHandlerTestCase(
        std::string name,
        ServiceConfig sv,
        LocationConfig lc,
        ResponseError err,
        Response expect,
        std::string bf = "");

    std::string title;
    ServiceConfig service;
    LocationConfig location;

    // owned resources
    std::unique_ptr<Socket> socket;
    std::unique_ptr<EventManager> events;
    std::unique_ptr<RequestContext> ctx;

    std::string body_file_path;
    ResponseError error;
    Response expected;
};

ErrorHandlerTestCase::ErrorHandlerTestCase(
    std::string name,
    ServiceConfig sv,
    LocationConfig lc,
    ResponseError err,
    Response expect,
    std::string bf)
    : title(name)
    , service(sv)
    , location(lc)
    , body_file_path(bf)
    , error(err)
    , expected(expect)
{
    socket = std::make_unique<Socket>(3, service); // 3 = dummy fd
    events = std::make_unique<EventManager>();
    ctx = std::make_unique<RequestContext>(*socket, *events, service);

    ctx->config().set(location);

    error.set(ctx.get());
}

std::vector<ErrorHandlerTestCase> generate_test_cases(void)
{
    // clang-format off
    std::vector<ErrorHandlerTestCase> test_cases;
    test_cases.reserve(100);

    // ------------------------------------------------------------------
    // Shared configs
    // ------------------------------------------------------------------

    // Config WITH a valid 404 error page on disk
    LocationConfig loc_with_404_page{
        "/error",
        {{Token::DirectiveMethods,   {"DELETE", "GET", "POST"}},
         {Token::DirectiveRoot,      {ERROR_FILES.c_str()}},
         {Token::DirectiveErrorPage, {"404", "./test_data/error/404.html"}},
        }
    };
    ServiceConfig svc_with_404_page{loc_with_404_page};

    // Config pointing at a non-existent error page path
    LocationConfig loc_missing_error_page{
        "/error",
        {{Token::DirectiveMethods,   {"DELETE", "GET", "POST"}},
         {Token::DirectiveRoot,      {ERROR_FILES.c_str()}},
         {Token::DirectiveErrorPage, {"404", "./test_data/error/does_not_exist.html"}},
        }
    };
    ServiceConfig svc_missing_error_page{loc_missing_error_page};

    // Config with NO error page configured at all
    LocationConfig loc_no_error_page{
        "/error",
        {{Token::DirectiveMethods, {"DELETE", "GET", "POST"}},
         {Token::DirectiveRoot,    {ERROR_FILES.c_str()}},
        }
    };
    ServiceConfig svc_no_error_page{loc_no_error_page};

    // ------------------------------------------------------------------
    // GOOD: custom error page exists on disk -> serve the file
    // ------------------------------------------------------------------

    test_cases.emplace_back(
        "404 with existing error page -> serves custom file",
        svc_with_404_page,
        loc_with_404_page,
        ResponseError{StatusCode::NotFound, "not found"},
        Response{StatusCode::NotFound, {}, {}},
        "./test_data/error/404.html"   // compare response body against this file
    );

    // ------------------------------------------------------------------
    // BAD 1: error page path configured but the file does not exist
    //        -> must fall back to the default generated HTML body
    // ------------------------------------------------------------------

    test_cases.emplace_back(
        "404 with missing error page file -> default HTML body",
        svc_missing_error_page,
        loc_missing_error_page,
        ResponseError{StatusCode::NotFound, "not found"},
        Response{StatusCode::NotFound, {}, ErrorHandler::make_default_body(StatusCode::NotFound)}
        // no body_file_path -> runner compares against expected.body()
    );

    // ------------------------------------------------------------------
    // BAD 2: no error page configured at all
    //        -> default generated HTML body for several status codes
    // ------------------------------------------------------------------

    test_cases.emplace_back(
        "404 no error page configured -> default HTML body",
        svc_no_error_page,
        loc_no_error_page,
        ResponseError{StatusCode::NotFound, "not found"},
        Response{StatusCode::NotFound, {}, ErrorHandler::make_default_body(StatusCode::NotFound)}
    );

    test_cases.emplace_back(
        "500 no error page configured -> default HTML body",
        svc_no_error_page,
        loc_no_error_page,
        ResponseError{StatusCode::InternalServerError, "internal server error"},
        Response{StatusCode::InternalServerError, {}, ErrorHandler::make_default_body(StatusCode::InternalServerError)}
    );

    test_cases.emplace_back(
        "403 no error page configured -> default HTML body",
        svc_no_error_page,
        loc_no_error_page,
        ResponseError{StatusCode::Forbidden, "forbidden"},
        Response{StatusCode::Forbidden, {}, ErrorHandler::make_default_body(StatusCode::Forbidden)}
    );

    test_cases.emplace_back(
        "400 no error page configured -> default HTML body",
        svc_no_error_page,
        loc_no_error_page,
        ResponseError{StatusCode::BadRequest, "bad request"},
        Response{StatusCode::BadRequest, {}, ErrorHandler::make_default_body(StatusCode::BadRequest)}
    );

    // ------------------------------------------------------------------
    // BAD 3: error page configured for 404 but error code is 500
    //        -> no matching page -> default HTML body
    // ------------------------------------------------------------------

    test_cases.emplace_back(
        "500 but only 404 page configured -> default HTML body",
        svc_with_404_page,
        loc_with_404_page,
        ResponseError{StatusCode::InternalServerError, "internal server error"},
        Response{StatusCode::InternalServerError, {}, ErrorHandler::make_default_body(StatusCode::InternalServerError)}
    );

    // clang-format on
    return test_cases;
}

void test_ErrorHandler(const ErrorHandlerTestCase& test)
{
    ErrorHandler handler{test.error};

    while (!handler.done())
    {
        handler.process();
    }

    const Response& res = handler.response();

    if (test.expected.status_code() == res.status_code())
    {
        if (test.body_file_path != "")
        // compare files
        {
            // get expected body
            std::ifstream in(test.body_file_path);
            std::ostringstream ss;
            ss << in.rdbuf();
            std::string file_content = ss.str();

            // get result body
            std::string res_body = utils::file_to_str(res.body_fd());
            if (file_content != res_body)
            {
                std::cout << constants::red << "[KO]! " << constants::reset << test.title
                          << " -> Content doesn't match: '" << test.body_file_path << "':\n\t" << file_content << "\n"
                          << "Response body: '\n\t" << res.body() << "'\n";
            }
            else
            {
                std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
            }
        }
        else
        // compare strings
        {
            if (test.expected.body() != res.body())
            {
                std::cout << constants::red << "[KO]! " << constants::reset << test.title
                          << " -> Content doesn't match:\n"
                          << "Expected:\n\t'" << test.expected.body() << "'Got:\n\t'" << res.body() << "'\n";
            }
            else
            {
                std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
            }
        }
    }
    else
    {
        std::cout << constants::red << "[KO]! " << constants::reset << test.title << " -> Status Codes don't match:\n"
                  << "Expected: " << test.expected.status_code() << "\nGot: " << res.status_code() << "\n";
    }
}

int main()
{
    Logger::set_global_level(Log::Fatal);

    std::cout << "==============================\n";
    std::cout << "======= ErrorHandler ========\n";
    std::cout << "==============================\n";

    std::vector<ErrorHandlerTestCase> tests = generate_test_cases();
    int stop = 0;
    for (const auto& test : tests)
    {
        try
        {
            test_ErrorHandler(test);
        }
        catch (const std::exception& e)
        {
            Logger::fatal("Test: '%s'", e.what());
        }
        if (stop == -1) // @NOTE: use this to test until a certain point
            break;
        ++stop;
    }
}
