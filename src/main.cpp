#include "Logger.hpp"
#include "Config.hpp"
#include <iostream>
//#include "HttpRequest.hpp"
//#include "HttpRequestParser.hpp"


const char *default_conf = "config/default.conf";

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		Logger::error("./webserv <configurations>.conf");
		return 1;
	}
	try
	{
		const char* file_path = argc == 2 ? argv[1] : default_conf;
		Config config(file_path);
		config.load();
		Logger::trace(config);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
	}

	//HttpRequestParser parser;

	//const char* raw_request =
	//	"GET /location HTTP/1.1\r\n"
	//	"Host: example.com\r\n"
	//	"Connection: keep-alive\r\n"
	//	"User-Agent: TestClient/1.0\r\n"
	//	"Content-Length: 11\r\n"
	//	"\r\n"
	//	"Hello World";
	//parser.feed(raw_request);
	//HttpRequest req = parser.get();
	//Logger::trace(req);
	//parser.clear();


	//const char* chunked_request =
	//	"POST /upload HTTP/1.1\r\n"
	//	"Host: example.com\r\n"
	//	"User-Agent: TestClient/1.0\r\n"
	//	"Transfer-Encoding: chunked\r\n"
	//	"\r\n"
	//	"8\r\n"
	//	"Chunked \r\n"
	//	"6\r\n"
	//	"Hello \r\n"
	//	"6\r\n"
	//	"World \r\n"
	//	"F\r\n"
	//	"MUAHAHAHAHHAHAA\r\n"
	//	"0\r\n"
	//	"\r\n";
	//parser.feed(chunked_request);
	//if (parser.error())
	//{
	//	std::cerr << "Error when parsing!\n";
	//}
	//else
	//{
	//	HttpRequest req = parser.get();
	//	Logger::trace(req);
	//}
	//parser.clear();

	//const char* chunked_request_with_trailing_headers =
	//	"POST /upload?id=123 HTTP/1.1\r\n"
	//	"Host: example.com\r\n"
	//	"User-Agent: TestClient/1.0\r\n"
	//	"Transfer-Encoding: chunked\r\n"
	//	"Trailer: Expires, X-Checksum\r\n"
	//	"\r\n"
	//	"8\r\n"
	//	"Chunked \r\n"
	//	"6\r\n"
	//	"Hello \r\n"
	//	"6;skip_ahead=chunk_extension\r\n"
	//	"World \r\n"
	//	"F\r\n"
	//	"MUAHAHAHAHHAHAA\r\n"
	//	"0\r\n"
	//	"Expires: Wed, 21 Oct 2025 07:28:00 GMT\r\n"
	//	"X-Checksum: deadbeef1234abcd\r\n"
	//	"\r\n";
	////parser.feed(chunked_request_with_trailing_headers);
	//size_t i = 0;
	//while ((!parser.done() || !parser.error()) && chunked_request_with_trailing_headers[i])
	//{
	//	parser.feed(chunked_request_with_trailing_headers[i]);
	//	i++;
	//}
	//if (parser.error())
	//{
	//	std::cerr << "Error when parsing!\n";
	//}
	//else
	//{
	//	HttpRequest req = parser.get();
	//	Logger::trace(req);
	//}
	//
	//Logger::debug("Main end");
	//return 0;
}
