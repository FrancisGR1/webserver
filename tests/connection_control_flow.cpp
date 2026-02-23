int main()
{

	// pré-ligação
	const char* file_path = argc == 2 ? argv[1] : constants::default_conf;
	Config config(file_path);
	config.load();

	const char* raw_request =
		"GET /static_files/ HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Connection: keep-alive\r\n"
		"User-Agent: TestClient/1.0\r\n"
		"Content-Length:    11 \r\n"
		"\r\n"
		"Hello World";

	//inicializados ao nível da ligação
	HttpRequestParser parser;
	HttpRequest request;
	NewHttpResponse response;
	HttpRequestContext context;

	// parsing
	parser.feed(raw_request);
	request = parser.get();

	//inicializar configuração de service
	ServiceConfig service_config(config.services[0]);
	//inicialiazar configuração do request
	HttpRequestConfig request_config(service);
	//resolução do request -> config, contexto
	http_utils::set_response_environment(request, service_config, request_config, context);
	//handler
	RequestDispatcher dispatcher;
	IRequestHandler* handler = dispatcher.dispatch(request, service_config);
	handler->process();
	if (handler->done())
	{
		NewHttpResponse response = handler->response();
		if (response.done())
		{
			//ligação fecha
		}
		else
		{
			int socket_fd = 123;
			response.send(socket_fd);
		}
	}

}
