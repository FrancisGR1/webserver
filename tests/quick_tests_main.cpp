#include <iostream>

#include "core/constants.hpp"
#include "core/Path.hpp"
#include "core/Logger.hpp"
#include "config/Config.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpRequestParser.hpp"
#include "http/CgiHandler.hpp"

//testes rápidos
int main(int argc, char *argv[])
{
	HttpRequestParser parser;

	const char* file_path = argc == 2 ? argv[1] : constants::default_conf;
	Config config(file_path);
	config.load();

	//abrir ficheiros com tests
	if (argc > 2)
	{
		Logger::error("./webserv <configurations_path>.conf");
		return 1;
	}
	try
	{
		std::cout << "TEST1: Simple GET\n";
		Logger::trace(config);


		const char* raw_request =
			"GET /static_files/ HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: keep-alive\r\n"
			"User-Agent: TestClient/1.0\r\n"
			"Content-Length:    11 \r\n"
			"\r\n"
			"Hello World";
		//		const char* chunked_request =
		//			"POST /upload HTTP/1.1\r\n"
		//			"Host: example.com\r\n"
		//			"User-Agent  : TestClient/1.0\r\n"
		//			"Transfer-Encoding: chunked\r\n"
		//			"\r\n"
		//			"8\r\n"
		//			"Chunked \r\n"
		//			"6\r\n"
		//			"Hello \r\n"
		//			"6\r\n"
		//			"World \r\n"
		//			"F\r\n"
		//			"MUAHAHAHAHHAHAA\r\n"
		//			"0\r\n"
		//			"\r\n";
		parser.feed(raw_request);
		std::cout << "Parser Error: " << std::boolalpha << parser.error() << "\n\n\n";
		HttpRequest req = parser.get();
		Logger::trace(req);
		HttpResponse res(req, config.services[1]);
		Logger::trace(res);
		parser.clear();

		std::cout << "TEST2: Chunked POST\n";
		const char* chunked_request =
			"POST /upload HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"User-Agent: TestClient/1.0\r\n"
			"Transfer-Encoding: chunked\r\n"
			"\r\n"
			"8\r\n"
			"Chunked \r\n"
			"6\r\n"
			"Hello \r\n"
			"6\r\n"
			"World \r\n"
			"F\r\n"
			"MUAHAHAHAHHAHAA\r\n"
			"0\r\n"
			"\r\n";
		parser.feed(chunked_request);
		if (parser.error())
		{
			std::cerr << "Error when parsing!\n";
		}
		else
		{
			HttpRequest req = parser.get();
			Logger::trace(req);
		}
		parser.clear();

		std::cout << "TEST3: Chunked POST\n";
		const char* chunked_request_with_trailing_headers =
			"POST /upload?id=123 HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"User-Agent: TestClient/1.0\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Trailer: Expires, X-Checksum\r\n"
			"\r\n"
			"8\r\n"
			"Chunked \r\n"
			"6\r\n"
			"Hello \r\n"
			"6;skip_ahead=chunk_extension\r\n"
			"World \r\n"
			"F\r\n"
			"MUAHAHAHAHHAHAA\r\n"
			"0\r\n"
			"Expires: Wed, 21 Oct 2025 07:28:00 GMT\r\n"
			"X-Checksum: deadbeef1234abcd\r\n"
			"\r\n";
		parser.feed(chunked_request_with_trailing_headers);
		size_t i = 0;
		while ((!parser.done() || !parser.error()) && chunked_request_with_trailing_headers[i])
		{
			parser.feed(chunked_request_with_trailing_headers[i]);
			i++;
		}
		if (parser.error())
		{
			std::cerr << "Error when parsing!\n";
		}
		else
		{
			HttpRequest req = parser.get();
			Logger::trace(req);
		}

		std::cout << "TEST4: CGI\n";
		Path script = "./root/script.py/users/42";
		CgiHandler cgi(parser.get(), config.services[0], script);

		parser.clear();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
	}


	return 0;
}
