#include <string>
#include <functional>
#include <vector>

#include "core/Path.hpp"

# define FILES_TO_TEST_DIR "test_path/"

struct TestCase {
	std::string title;
	Path path;
	std::function<bool(const Path&)> expect;
};

//@TODO implementar
//cgi
//ficheiro
//diretório
//permissões
//existe

void test_path_resolution(const TestCase& test);

std::vector<TestCase> generate_test_cases(void)
{
	std::vector<TestCase> test_cases =
	{
		// --- Basic matching ---
		// tc 1: simple match
		{
			.title= "simple existant path",
			.path= Path(FILES_TO_TEST_DIR"simple"),
			.expected= "./root/get/file.html"
		},
		// tc 2: no match returns target as-is
		{
			.title= "no matching location returns target",
			.service= ServiceConfig({
					LocationConfig("/get",  "./root"),
					LocationConfig("/post", "./root"),
					}),
			.target= "/unknown/file.html",
			.expected= "/unknown/file.html"
		},

		// --- Longest prefix match ---
		// tc 3: longest prefix wins over shorter one
		{
			.title= "longest prefix match wins",
			.service= ServiceConfig({
					LocationConfig("/api",    "./root"),
					LocationConfig("/api/v1", "./root"),
					}),
			.target= "/api/v1/users",
			.expected= "./root/api/v1/users"
		},
		// tc 4: falls back to shorter prefix
		{
			.title= "falls back to shorter prefix when no longer match",
			.service= ServiceConfig({
					LocationConfig("/api",    "./root"),
					LocationConfig("/api/v1", "./root"),
					}),
			.target= "/api/v2/users",
			.expected= "./root/api/v2/users"
		},

		// --- Trailing slash ---
		// tc 5: target with trailing slash
		{
			.title= "target with trailing slash",
			.service= ServiceConfig({
					LocationConfig("/get", "./root"),
					}),
			.target= "/get/",
			.expected= "./root/get/"
		},
		// tc 6: target without trailing slash
		{
			.title= "target without trailing slash",
			.service= ServiceConfig({
					LocationConfig("/get", "./root"),
					}),
			.target= "/get",
			.expected= "./root/get"
		},
		// tc 7: nested path with trailing slash
		{
			.title= "nested path with trailing slash",
			.service= ServiceConfig({
					LocationConfig("/get", "./root"),
					}),
			.target= "/get/subdir/",
			.expected= "./root/get/subdir/"
		},

		// --- Root location ---
		// tc 8: root target with no matching location
		{
			.title= "root target no match",
			.service= ServiceConfig({
					LocationConfig("/", "./root")
					}),
			.target= "/hello.html",
			.expected= "./root/hello.html"
		},
		// tc 9: root with nonexistant target
		{
			.title= "root with nonexistant target",
			.service= ServiceConfig({
					LocationConfig("/", "./root"),
					}),
			.target= "/anything/here",
			.expected= "./root/anything/here"
		},
	};

	return test_cases;
}

int main()
{
	Logger::set_global_level(Log::Trace);

	std::vector<TestCase> tests = generate_test_cases();
	for (auto& test : tests)
	{
		//std::cerr << "\n-------\nService: " << test.service << "\n-----\n";
		test_Path(test);
	}
	return 0;
} 

void test_Path(const TestCase& test)
{
	const LocationConfig* location = NULL;
	// get resolved path
	std::string result = RequestProcessor::resolve_path(test.target, test.service, location);

	// check
	if (result == test.expected)
	{
		std::cout << constants::green << "[OK] " << constants::reset << test.title << "\n";
	}
	else
	{
		std::cerr << constants::red << "[KO]! " << constants::reset << test.title << "\n";
		//std::cerr << "\n-------\nService: " << service << "\n-----\n";
		std::cerr << "\tGot: " << result << "\n" << "\tExpected: " << test.expected;
		std::cerr << "\n\n";
	}
}
