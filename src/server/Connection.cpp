#include "Connection.hpp"
#include "Webserver.hpp"
#include <cstdlib>

Connection::Connection(int sock, const ServiceConfig& service, EventManager& events) 
	: sock_(sock)
	, bytes_sent_(0)
	, state_(Receiving)
	, service_(service)
	, events_(events)
	, processor_(service, events) {}


Connection::~Connection() {}

void	Connection::setResponse(const std::string &response)
{
	resp_ = response;
}

/* member fuctions */

// quando chama process():
//
// void Connection::process()
// {
// 	switch(state_)
// 	{
// 		case Receiving:
// 		{
// 			// @TODO: feed buffer
// 			if (parser_.done())
// 			{
// 				request_ = parser.get();
// 				processor.set(request_);
// 				state_ = Processing;		
// 			}
// 			break;
// 		}
// 		case Processing:
// 		{
// 			processor_.process();
// 			if (processor_.done())
// 			{
// 				response_ = processor.get();
// 				state_ = Sending;		
// 			}
// 			break;
// 		}
// 		case Sending:
// 		{
// 			// @QUESTION: apanhar n bytes do send()? 
// 			response_.send(socket.fd);
// 			if (response_.done())
// 			{
// 				state_ = Done;		
// 			}
// 			break;
// 		}
// 		default: 
// 			break;
// 	}
// }

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
	if (bytes_sent_ >= resp_.length())
		return (true);
	
	ssize_t sent = send(sock_, resp_.c_str() + bytes_sent_, resp_.length() - bytes_sent_, 0);
	if (sent <= 0)
		return (false);
	
	bytes_sent_ += sent;
	/* terminou */
	if (bytes_sent_ >= resp_.length())
		return (true);
	/* ainda falta para enviar */
	return (false);
}
