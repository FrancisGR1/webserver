#include "Connection.hpp"
#include "Webserver.hpp"
#include <cstdlib>

Connection::Connection(int sock) : sock_(sock)
{ 

}

Connection::~Connection()
{
	if (sock_ >= 0)
		close(sock_);
}

/* getters and setters */

int	Connection::getSock()
{ 
	return (sock_);
}

void	Connection::setSock(int sock)
{
	sock_ = sock;
}

std::string	&Connection::getRequest()
{
	return (request_);
}

void	Connection::setRequest(const std::string &request)
{
	request_ = request;
}

std::string	&Connection::getResponse()
{
	return (response_);
}

void	Connection::setResponse(const std::string &response)
{
	response_ = response;
}

/* member fuctions */

void	Connection::readRequest()
{
	char	buffer[4096];
	ssize_t	bytes;

	/* ler header completo */
	while ((bytes = recv(sock_, buffer, sizeof(buffer), 0)) > 0)
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
		bytes = recv(sock_, buffer, sizeof(buffer), 0);
		if (bytes <= 0)
			break ;
		request_.append(buffer, bytes);
		body_received += bytes;
	}
}

void	Connection::handle()
{
	/* apos aceitar a conexao com o client, leio o que o foi passado */
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
	send(sock_, response.c_str(), response.length(), 0);
}
