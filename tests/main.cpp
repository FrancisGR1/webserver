#include <iostream>

#include "core/constants.hpp"
#include "core/Logger.hpp"
#include "config/Config.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpRequestParser.hpp"

//class Test
//{
//	public:
//		bool valid_request();
//		bool valid_response();
//
//	private:
//		std::string m_group_name;
//		std::string m_name;
//
//		std::string m_raw_request;
//
//		HttpRequest m_parsed_request;
//		HttpResponse m_response;
//
//		std::string m_expected_response;
//		http::request<http::string_body> m_expected_parsed_request;
//};
//
//enum class ParseState
//{ 
//	TestSetTitle = 0, // #
//	TestTitle,        // ## 
//	Comment,          // ###
//	Request,
//	ExpectedResponse,
//	END
//};
//
//std::vector<Test> load_tests(std::stringstream &file_content)
//{
//	std::vector<Test> test_data;
//	Test test;
//
//	ParseState state = ParseState::Title;
//	std::string line;
//
//	while (std::getline(file_content, line))
//	{
//		switch (state)
//		{
//			case ParseState::TestTitle: 
//			{
//				size_t pos = line.find_first_not_of("# \t");
//				test.test_name = line.substr(pos);
//				state = ParseState::Request;
//				continue;
//			}
//			case ParseState::Request: 
//			{
//				test.http_request += line + "\n";
//				if (line.empty())
//				{
//					test_data.push_back(test);
//					test.test_name = "";
//					test.http_request = "";
//					state = ParseState::Title;
//				}
//				continue;
//			}
//			case ParseState::ExpectedResponse:
//			{
//			}
//			default: {} 
//		}
//	}
//	test_data.push_back(test);
//	return (test_data);
//}
//
//int main(int argc, char *argv[])
//{
//	for (int arg = 1; arg < argv; ++arg)
//	{
//		std::vector<Test> tests = load_tests(argv[arg]);
//	}
//}

