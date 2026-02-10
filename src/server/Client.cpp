#include "Client.hpp"
#include "Webserver.hpp"
#include <cstdlib>

Client::Client(int client_fd) : client_fd_(client_fd)
{ 

}

Client::~Client()
{
	if (client_fd_ >= 0)
		close(client_fd_);
}

/* getters and setters */

int	Client::getClient_fd()
{ 
	return (client_fd_);
}

void	Client::setClient_fd(int client_fd)
{
	client_fd_ = client_fd;
}

std::string	&Client::getRequest()
{
	return (request_);
}

void	Client::setRequest(const std::string &request)
{
	request_ = request;
}

std::string	&Client::getResponse()
{
	return (response_);
}

void	Client::setResponse(const std::string &response)
{
	response_ = response;
}

/* member fuctions */

void	Client::readRequest()
{
	char	buffer[4096];
	ssize_t	bytes;

	/* ler header completo */
	while ((bytes = recv(client_fd_, buffer, sizeof(buffer), 0)) > 0)
	{
		request_.append(buffer, bytes);
		if (request_.find("\r\n\r\n") != std::string::npos)
			break ;
	}

	if (request_.empty())
		return ;
	/* se existir Content-Length, eu salvo o valor dele */
	size_t	pos = request_.find("Content-Length:");
	size_t	content_length = 0;
	if (pos != std::string::npos)
	{
		pos += 15;
		while (request_[pos] == ' ')
			pos++;
		content_length = std::atoi(request_.c_str() + pos);
	}
	/* ler o body se ja nao tiver sido lido todo */
	size_t	header_end = request_.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return ;

	size_t	body_start = header_end + 4;
	size_t	body_received = request_.size() - body_start;
	while (body_received < content_length)
	{
		bytes = recv(client_fd_, buffer, sizeof(buffer), 0);
		if (bytes <= 0)
			break ;
		request_.append(buffer, bytes);
		body_received += bytes;
	}
}

void	Client::handle()
{
	/* apos aceitar o client, leio o que o foi passado */
	readRequest();

	/* aqui fazer o parser do request */
	
	std::cout << "Resquest received:\n";
	std::cout << request_ << std::endl;
	/* ignoro o request e apenas respondo o client com um simples "Hello World!" usando send */
	std::string response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 13\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"Hello World!\n";
	send(client_fd_, response.c_str(), response.length(), 0);
}
