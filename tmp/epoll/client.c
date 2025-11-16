#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <stdbool.h>

int connect_to_server(const char *host, const char *port)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));

	struct addrinfo *peer_address;
	getaddrinfo(host, port, &hints, &peer_address);

	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
	printf("Remote address: %s %s\n", address_buffer, service_buffer);

	printf("Creating socket\n");
	int server_fd = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	if (server_fd < 0)
	{
		fprintf(stderr, "Error: invalid socket: %d\n", errno);
		return -1;
	}

	printf("Connecting...\n");
	int connect_res = connect(server_fd, peer_address->ai_addr, peer_address->ai_addrlen);
	if (connect_res != 0)
	{
		fprintf(stderr, "Error: invalid socket: %d\n", errno);
		return -1;
	}

	freeaddrinfo(peer_address);

	printf("Success: connected\n");

	return server_fd;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "./client <host> <port>\n");
		return 1;
	}

	//ligar ao servidor
	const char *host = argv[1];
	const char *port = argv[2];
	int server_fd = connect_to_server(host, port);
	if (server_fd < 0)
		return 1;

	while (true)
	{
		//enviar mensagem
		char msg[2000];
		printf("Send message: ");
		scanf("%s", msg);
		send(server_fd, msg, strlen(msg), 0);
	}
}
