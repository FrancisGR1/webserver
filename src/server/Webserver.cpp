#include "Webserver.hpp"
#include "Connection.hpp"
#include <sstream>

bool	Webserver::is_running = true;

Webserver::Webserver(const Config &config) : config_(config) {}

Webserver::~Webserver() {}

void	Webserver::log(const std::string &message)
{
	std::cout << message << std::endl;
}

void	Webserver::addEpoll(const int socket, uint32_t events)
{
	epoll_event event = {};
	event.events = events;
	event.data.fd = socket;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket, &event) == -1)
		log("Error: epoll_ctl ADD failed!");
}

void	Webserver::modifyEpoll(int socket, uint32_t events)
{
	epoll_event event = {};
	event.events = events;
	event.data.fd = socket;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket, &event) == -1)
		log("Error: epoll_ctl MOD failed!");
}

void	Webserver::removeEpoll(int socket)
{
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket, NULL) == -1)
		log("Error: epoll_ctl DEL failed!");
}
/*	- Criar o epoll
	- Criar todos os sockets de servidor (listen)
	- Configurar eles corretamente
	- Registrar eles no epoll 
*/
int	Webserver::setupSocket()
{
	/* cria epoll */
	epoll_fd_ = epoll_create(1);
	if (epoll_fd_ == -1)
		return (log("Error: Failed to create epoll fd!"), 1);
	/* percorrer todos os services */
	for(size_t i = 0; i < config_.services.size(); i++)
	{
		const ServiceConfig& service = config_.services[i];
		/* percorrer todos os listeners desse service */
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
			
			int flags = fcntl(sock, F_GETFL, 0);
			if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
				return (freeaddrinfo(result), log("Error: Failed to set client socket to non-blocking mode!"), 1);
			
			if (bind(sock, result->ai_addr, result->ai_addrlen) < 0)
				return (freeaddrinfo(result), log("Error: bind configuration!"), 1);

			if (listen(sock, 10) < 0)
				return (freeaddrinfo(result), log("Error: listen failed!"), 1);
			/* adicionar o sock na lista e liberar o result */
			server_sockets.push_back(sock);
			addEpoll(sock, EPOLLIN);
			freeaddrinfo(result);
			std::cout << "Lisntening on " << listener.host << ":" << listener.port << "\n";
		}
	}
	return (0);
}

bool	Webserver::isServerSocket(int fd)
{
	for (size_t i = 0; i < server_sockets.size(); i++)
	{
		if (server_sockets[i] == fd)
			return (true);
	}
	return (false);
}

void	Webserver::acceptConnection(const int sock)
{
	int client_sock = accept(sock, NULL, NULL);
	if (client_sock < 0)
	{
		log("Erro: Failed to accept the client connection!");
		Webserver::is_running = false;
		return ;
	}
	int opt = 1;
	if (setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		log("Error: Socket configuration!");
	
	int flags = fcntl(client_sock, F_GETFL, 0);
	if (flags == -1 || fcntl(client_sock, F_SETFL, flags | O_NONBLOCK) == -1)
		log("Error: Failed to set client socket to non-blocking mode!");
	
	connections[client_sock] = Connection(client_sock);
	if (client_sock != -1)
		addEpoll(client_sock, EPOLLIN);
	std::cout << "Cliente adicionado!\n";
}

void	Webserver::handleConnection(const int sock, epoll_event event)
{
	/* client - leitura */
	if (event.events & EPOLLIN)
	{
		Connection& conn = connections[sock];
		if (!conn.readRequest())
		{
			removeEpoll(sock);
			close(sock);
			connections.erase(sock);
			return ;
		}
		if (conn.isReady())
		{
			/* resposta simples */
			conn.setResponse(
				"HTTP/1.0 200 OK\r\n"
				"Content-Length: 13\r\n"
				"Content-Type: text/plain\r\n"
				"\r\n"
				"Hello World!\n"
			);
			/* mudar para POLLOUT */
			modifyEpoll(sock, EPOLLOUT);
		}
	}
	/* client - escrita */
	else if (event.events & EPOLLOUT)
	{
		Connection& conn = connections[sock];
		if (conn.sendResponse())
		{
			removeEpoll(sock);
			close(sock);
			connections.erase(sock);
		}
	}
}

void	Webserver::startServer()
{
	/* etapa 1 - iniciar e configurar sockets do servidor  */
	if (setupSocket())
		return ;
	
	/* etapa 2 - loop server */
	while (is_running)
	{
		int num_events = epoll_wait(epoll_fd_, events_, 10, -1);
		if (num_events == -1)
		{
			log("Erro: Failed to wait on epoll!");
			continue ;
		}

		for (int i = 0; i < num_events; i++)
		{
			int sock = events_[i].data.fd;
			/* erro */
			if (events_[i].events & (EPOLLERR | EPOLLHUP))
			{
				removeEpoll(sock);
				close(sock);
				connections.erase(sock);
				continue ;
			}
			/* servidor */
			if (isServerSocket(sock) && (events_[i].events & EPOLLIN))
				acceptConnection(sock);
			else
				handleConnection(sock, events_[i]);
		}
	}
}
 /* fazer parser da request para criar response */