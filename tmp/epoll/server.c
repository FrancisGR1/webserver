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

#define MAX_EVENTS 10


int create_socket(const char *host, const char *port)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_address;
	getaddrinfo(host, port, &hints, &bind_address);

	int socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if (socket_listen < 0)
	{
		fprintf(stderr, "Error: socket(): %d\n", errno);
		return -1;
	}

	int bind_result = bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen);
	if (bind_result != 0)
	{
		fprintf(stderr, "Error: bind(): %d\n", errno);
		return -1;
	}

	printf("Creating socket:\n\tHost: 0\n\tPort: %s\n", port);
	freeaddrinfo(bind_address);

	int listen_result = listen(socket_listen, 20);
	if (listen_result < 0)
	{
		fprintf(stderr, "Error: listen(): %d\n", errno);
		return -1;
	}
	return socket_listen;
}


int main()
{
	//socket
	int socket_fd = create_socket(0, "8080");
	if (socket_fd < 0)
		return -1;

	//epoll - criar instância
	int epoll_fd = epoll_create1(0);

	//epoll - inicialização
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = socket_fd;

	const int max_events = 10;
	struct epoll_event events[max_events];

	//epoll - adicionar socket_fd à lista
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);

	while (true)
	{
		//epoll - esperar que um evento ocorra
		int events_size = epoll_wait(epoll_fd, events, max_events, -1);
		if (events_size < 0)
		{
			fprintf(stderr, "Error: epoll_wait(): %d\n", errno);
			return 1;
		}
		for (int i = 0; i < events_size; i++)
		{
			if (events[i].data.fd == socket_fd)
			{
				//aceitar o evento da nova ligação
				struct sockaddr_storage client_addr;
				socklen_t client_len = sizeof(struct sockaddr);
				int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
				if (client_fd < 0)
				{
					fprintf(stderr, "Error: accept(): %d\n", errno);
					return 1;
				}

				//adicionar o evento de ligação à lista de eventos a ser observados
				struct epoll_event client_ev;
				client_ev.events = EPOLLIN;
				client_ev.data.fd = client_fd;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);

				//mostrar mensagem
				printf("New client: %d\n", client_fd);
			}
			else
			{
				//ler mensagem
				int client_fd = events[i].data.fd;
				size_t buf_len = 40;
				char buf[buf_len];
				ssize_t bytes_read = recv(client_fd, buf, buf_len, 0);
				if (bytes_read <= 0)
				{
					printf("Client %d disconnected\n", client_fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
					continue;
				}
				buf[bytes_read] = '\0';
				printf("%s\n", buf);
			}
		}
	}
	close(socket_fd);
}
