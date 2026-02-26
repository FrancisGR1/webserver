#include "Connection.hpp"
#include "Webserver.hpp"
#include <cstdlib>

Connection::Connection() : bytes_sent_(0), state_(Receiving) {}

Connection::Connection(int sock, const ServiceConfig& service, const EventManager& events) 
: sock_(sock)
, bytes_sent_(0)
, state_(Receiving)
, processor_(sock.service, events)
, events_(events)  {}

Connection::~Connection() {}

/* getters and setters */

// int	Connection::getSock()
// { 
// 	return (sock_);
// }

// void	Connection::setSock(int sock)
// {
// 	sock_ = sock;
// }

// HttpRequest	&Connection::getRequest()
// {
// 	// return (parser_.get());
// }

// void	Connection::setRequest(const std::string &request)
// {
// 	// parser_ = request;
// }

// std::string	&Connection::getResponse()
// {
// 	return (response_);
// }

void	Connection::setResponse(const std::string &response)
{
	response_ = response;
}

/* member fuctions */

// quando chama process():
//
void Connection::process()
{
	switch(state_)
	{
		case Receiving:
		{
			// @TODO: feed buffer
			if (parser_.done())
			{
				request_ = parser.get();
				processor.set(request_);
				state_ = Processing;		
			}
			break;
		}
		case Processing:
		{
			processor_.process();
			if (processor_.done())
			{
				response_ = processor.get();
				state_ = Sending;		
			}
			break;
		}
		case Sending:
		{
			// @QUESTION: apanhar n bytes do send()? 
			response_.send(socket.fd);
			if (response_.done())
			{
				state_ = Done;		
			}
			break;
		}
		default: 
			break;
	}
}

bool Connection::done() const
{
	return state_ == Done;
}

bool	Connection::readRequest()
{
	char	buffer[4096];

	ssize_t	bytes = recv(sock_, buffer, sizeof(buffer), 0);
	/* client fechou ou erro */
	if (bytes <= 0)
		return (false);

	//@QUESTION: isto funciona?
	parser_.feed(buffer);
	// for (ssize_t i = 0; i < bytes; i++)
	// 	parser_.feed(buffer[i]);

	return (true);
}

bool	Connection::isReady() const
{
    return (parser_.done());
}

bool	Connection::sendResponse()
{
	if (bytes_sent_ >= response_.length())
		return (true);
	
	ssize_t sent = send(sock_, response_.c_str() + bytes_sent_, response_.length() - bytes_sent_, 0);
	if (sent <= 0)
		return (false);
	
	bytes_sent_ += sent;
	/* terminou */
	if (bytes_sent_ >= response_.length())
		return (true);
	/* ainda falta para enviar */
	return (false);
}
