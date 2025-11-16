#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

namespace http = boost::beast::http;

struct test_case
{
	std::string test_name;
	std::string http_request;
};

enum class ParseState
{ 
	IN_TITLE = 0, 
	IN_REQUEST,
	END
};

std::vector<test_case> load_tests(std::stringstream &file_content)
{
	std::vector<test_case> test_data;
	test_case tc;

	ParseState state = ParseState::IN_TITLE;
	std::string line;

	size_t idx = 0;
	while (std::getline(file_content, line))
	{
		switch (state)
		{
			case ParseState::IN_TITLE: 
			{
				size_t pos = line.find_first_not_of("# \t");
				tc.test_name = line.substr(pos);
				state = ParseState::IN_REQUEST;
				continue;
			}
			case ParseState::IN_REQUEST: 
			{
				tc.http_request += line + "\n";
				if (line.empty())
				{
					test_data.push_back(tc);
					tc.test_name = "";
					tc.http_request = "";
					state = ParseState::IN_TITLE;
				}
				continue;
			}
		}
	}
	test_data.push_back(tc);
	return (test_data);
}

int main() {
	// Step 1: Raw HTTP request data (like from a socket)
	std::string file_path("test_http_parser_GET.md");
	std::ifstream file(file_path.c_str());
	if (!file.is_open()) {
		std::cerr << "Error: Can't open: " << file_path << std::endl;
	}
	std::stringstream buf;
	buf << file.rdbuf();

	// Step 2: Passar todos os testes para um vetor de testes
	std::vector<test_case> tests = load_tests(buf);
	if (tests.empty())
		std::cout << "test empty!\n";
	size_t idx = 0;
	for (auto test : tests)
	{
		std::cout << idx++ << " ";
		std::cout << test.test_name << std::endl << test.http_request << std::endl;
	}
	// Step 3: Make a parser for HTTP requests
	// @TODO: colocar o que está em baixo num loop; passar todos os std::string do vetor para o request_parser
		std::string request = buf.str();
		http::request_parser<http::string_body> parser;
	
		// Step 3: Feed the data to the parser
		boost::system::error_code ec;
		parser.put(boost::asio::buffer(tests[0].http_request), ec);
	
		if (ec && ec != http::error::need_more) {
			std::cerr << "Parse error: " << ec.message() << std::endl;
			return 1;
		}
	
		// Step 4: Extract the parsed request
		http::request<http::string_body> req = parser.get();
	
		// Step 5: Use it!
		std::cout << "Method: " << req.method_string() << "\n";
		std::cout << "Target: " << req.target() << "\n";
		std::cout << "Version: " << (req.version() == 11 ? "HTTP/1.1" : "HTTP/1.0") << "\n";
	
		std::cout << "Headers:\n";
		for (auto const& field : req)
			std::cout << "  " << field.name_string() << ": " << field.value() << "\n";
	
		std::cout << "Body: " << req.body() << "\n";
}
