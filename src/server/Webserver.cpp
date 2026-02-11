#include "Webserver.hpp"
#include "Connection.hpp"
#include <poll.h>

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
	/* socket pedir ao sistema operacional um canal para comunicacao em rede */
	/* retona um int (fd) arquivo especial */
	/* socket e tratado como arquivo no Linux */
	/* arg 1: domain (tipo de rede):			AF_INET = IPv4 */
	/* arg 2: type (como os dados trafegam):	SOCK_STREAM = TCP */
	/* arg 3: protocol (qual protocolo exato):	unico possivel para esse (TCP) = 0 */
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
		return (log("Error: socket creation!"), 1);
	/* setsockopt = configura opcoes do socket */
	/* arg 1: (fd) do socket que irei configurar */
	/* arg 2: (level) 	SOL_SOCKET = opcoes gerais do socket */
	/* arg 3: (optname)	SO_REUSEADDR = para permitir reutilizar essa porta imediatamente */
	/* arg 4: (optval) o valor da opcao = 1 para ativado */
	/* arg 5: (optlen) tamanho de optval = sizeof(optval) */
	opt_value = 1;
	socket_config = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));
	if (socket_config < 0)
		return (log("Error: socket configuration!"), 1);
	/* bind() associa o socket a um IP e a uma porta. */
	addr = sockaddr_in();
	addr.sin_family = AF_INET; /* deve ser o mesmo do socket (IPv4) */
	addr.sin_port = htons(8080); /* precisar estar em "network byte order" padrao htons (host to network short) */
	addr.sin_addr.s_addr = INADDR_ANY; /* define em qual IP o servidor aceita conexoes, sendo INADDR_ANY para aceitar qualquer IP */
	if (bind(server_socket, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
		return (log("Error: bind configuration!"), 1);
	/* listen  para transformar o socket em um servidor pronto para aceitar conexoes */
	if (listen(server_socket, 10) < 0)
		return (log("Error: listen failed!\n"), 1);
	std::cout << "Listening...\n";
	return (0);
}

void	Webserver::startServer()
{
	if (setupSocket())
		return ;
	/* loop server */
	while (is_running)
	{
		/* socket para transmissao de dados com client*/
		/* salvando endereco do client */
		struct sockaddr_in	client_addr = {};
		socklen_t			client_len = sizeof(client_addr);
		int 				client_sock = accept(server_socket, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
		/* accept e bloqueante, entao ele fica aguardando um client para aceitar a conexao */
		/* nao sendo possivel fazer nada enquanto ele aguarda, entao devo usar select ou poll para evitar isso */
		if (client_sock < 0)
		{
			log("Erro: Failed to accept the client connection!");
			continue ;
		}
		std::cout << "Client connected!\n";
		Connection conn(client_sock);
		conn.handle();
	}
	close(server_socket);
}
