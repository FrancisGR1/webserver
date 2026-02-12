#include "Webserver.hpp"
#include "Connection.hpp"

bool	Webserver::is_running = true;

Webserver::Webserver(const Config &config) : config_(config)
{
	//std::string host = config_.services[0].listeners[0].host;
	//size_t port = config_.services[0].listeners[0].port;
}

Webserver::~Webserver()
{

}

void	Webserver::log(const std::string &message)
{
	std::cout << message << std::endl;
}

int	Webserver::setupSocket()
{
	/* percorrer todos os services */
	for(size_t i = 0; i < config_.services.size(); i++)
	{
		const ServiceConfig& service = config_.services[i];
		/* percorrer todo os listeners desse service */
		for (size_t j = 0; j < service.listeners.size(); j++)
		{
			const Listener& listener = service.listeners[j];
			
			struct addrinfo hints = {};
			struct addrinfo* result;

			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE;

			if (getaddrinfo(listener.host.c_str(), listener.port.c_str(), &hints, &result) != 0)
				return (log("Error: getaddrinfo!"), 1);
			/* criar socket, configurar, bind... */
			int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (sock < 0)
				return (freeaddrinfo(result), log("Error: socket creation!"), 1);

			int opt = 1;
			if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
				return (freeaddrinfo(result), log("Error: socket configuration!"), 1);
			
			if (bind(sock, result->ai_addr, result->ai_addrlen) < 0)
				return (freeaddrinfo(result), log("Error: bind configuration!"), 1);

			if (listen(sock, 10) < 0)
				return (freeaddrinfo(result), log("Error: listen failed!"), 1);
			/* adicionar o sock na lista e liberar o result */
			server_sockets.push_back(sock);
			freeaddrinfo(result);
			std::cout << "Lisntening on " << listener.host << ":" << listener.port << "\n";
		}
		
	}
	return (0);
}

void	Webserver::createPoll()
{
	for (size_t i = 0; i < server_sockets.size(); i++)
	{
		pollfd poll;
		poll.fd = server_sockets[i];
		poll.events = POLLIN;
		poll.revents = 0;
		fds.push_back(poll);
	}
}

void	Webserver::startServer()
{
	if (setupSocket())
		return ;
	
	createPoll();
	
	/* loop server */
	while (is_running)
	{
		/* iniciando a poll */
		if (poll(fds.data(), fds.size(), -1) <= 0)
			continue ;
		/* percorrer a poll de fds */
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				/* socket para transmissao de dados com client*/
				int client_sock = accept(fds[i].fd, NULL, NULL);
				if (client_sock < 0)
				{
					log("Erro: Failed to accept the client connection!");
					Webserver::is_running = false;
				}
				std::cout << "Client connected!\n";
				Connection conn(client_sock);
				conn.handle();
			}
		}
	}
	/* fechar server_sockets */
}
