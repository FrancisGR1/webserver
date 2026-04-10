#include <fstream>
#include <string>

#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "http/StatusCode.hpp"
#include "http/processor/handler/DeleteHandler.hpp"
#include "http/response/ResponseError.hpp"

#include "test_utils.hpp"

static const std::string DELETE_FILES = "./test_data/delete/";

std::vector<tu::HandlerTestCase> generate_test_cases(void)
{
    // clang-format off
    std::vector<tu::HandlerTestCase> test_cases;
    test_cases.reserve(100);

    // default service
    LocationConfig del{
        "/delete",
        {{Token::DirectiveMethods, {"DELETE"}},
         {Token::DirectiveRoot, {DELETE_FILES.c_str()}},
    }
    };
    ServiceConfig service{del};

    // create file to be deleted
    const std::string delete_me_md = DELETE_FILES + "delete_me.md";
    std::ofstream(delete_me_md.c_str()).close();

    // GOOD
    test_cases.emplace_back(
        "simple delete",
        delete_me_md,
        service,
        del,
        Request( "POST", "/delete/", "", "HTTP/1.1", {}, {}, StatusCode::Ok),
        Response( StatusCode::NoContent, {}, {})
	);

    // BAD
    // 1. file doesn't exist
    test_cases.emplace_back(
        "delete non-existent file",
        DELETE_FILES + "does_not_exist.md",
        service,
        del,
        Request("POST", "/delete/", "", "HTTP/1.1", {}, {}, StatusCode::Ok),
        Response(StatusCode::NotFound, {}, {})
    );

    // 2. no permission
    // create subdir
    const std::string no_perm_dir = DELETE_FILES + "restricted_zone/";
    mkdir(no_perm_dir.c_str(), 0777); 
    // create file
    const std::string no_perm_file = no_perm_dir + "no_permission.md";
    std::ofstream(no_perm_file.c_str()).close();
    // remove write and execute perms
    chmod(no_perm_dir.c_str(), 0555); 
    
    test_cases.emplace_back(
        "delete file without permission",
        no_perm_file,
        service,
        del,
        Request("POST", "/delete/", "", "HTTP/1.1", {}, {}, StatusCode::Ok),
        Response(StatusCode::Forbidden, {}, {})
    );

    // 3. target is a directory
    test_cases.emplace_back(
        "delete a directory",
        DELETE_FILES,
        service,
        del,
        Request("POST", "/delete/", "", "HTTP/1.1", {}, {}, StatusCode::Ok),
        Response(StatusCode::Conflict, {}, {})
    );
    // clang-format on

    return test_cases;
}

void test_DeleteHandler(const tu::HandlerTestCase& test)
{
    Logger::info("===============\nTest: '%s'", test.title.c_str());
    DeleteHandler handler{test.request, *test.ctx};
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
            Logger::error("ResponseError: '%s'", error.msg().data());
            if (error.status_code() == test.expected.status_code())
            {
                std::cerr << constants::green << "[OK] " << constants::reset << test.title << "\n";
            }
            else
            {
                std::cout << constants::red << "[KO]! " << constants::reset << test.title
                          << "\n\tExpected code: " << test.expected.status_code()
                          << "\n\tGot code: " << error.status_code() << "\n";
            }
            return;
        }
    }

    // delete response
    Response res = handler.response();
    Logger::debug_obj(res, "GetHandler: response: ");

    int eliminated = !tu::file_exists(test.file_path.raw.c_str());
    if (eliminated && res.status_code() == test.expected.status_code())
    {
        std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
    }
    else
    {
        // clang-format off
        std::cout << constants::red << "[KO]! " << constants::reset << test.title
                  << "\n\tExpected code: " << test.expected.status_code() 
		  << "\n\tGot code: " << res.status_code()
                  << "\n\tEliminated file: '" << test.file_path.raw << "' -> " << std::boolalpha << eliminated << std::endl;
        // clang-format on
    }
}

int main()
{
    Logger::set_global_level(Log::Warn);

    Logger::trace("=================");
    Logger::info("DELETE HANDLER START");

    std::cout << "\n===== DELETE tests====\n";

    std::cout << "\nGood\n";

    std::vector<tu::HandlerTestCase> tests = generate_test_cases();
    int stop = 0;
    for (auto& test : tests)
    {
        try
        {
            test_DeleteHandler(test);
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
