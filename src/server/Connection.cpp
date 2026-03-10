#include "http/request/Request.hpp"
#include "http/processor/RequestProcessor.hpp"
#include "core/constants.hpp"
#include "core/Logger.hpp"
#include "Socket.hpp"
#include "Connection.hpp"
#include "Webserver.hpp"

Connection::Connection(Socket& conn_socket, EventManager& events) 
	: work_state_(Receiving)
	, socket_(conn_socket)
	, service_(conn_socket.service())
	, events_(events)
	, processor_(conn_socket, events) 
{
	Logger::trace("Create connection: %d\n", socket_.fd());
}


Connection::~Connection() 
{
	Logger::trace("Destroy connection: %d\n", socket_.fd());
}

void Connection::work(const epoll_event& on_event)
{
	//@TODO: check if event matches connection state
	(void) on_event;

	std::cout << "working!\n";
	switch(work_state_)
	{
		case Receiving:
			{
				// read client info to buffer
				char buffer[constants::read_chunk_size];
				ssize_t bytes = recv(socket_.fd(), buffer, sizeof(buffer), 0);
				if (bytes <= 0)
				{
					//@TODO
					break;
				}

				// parser works on raw strings but just to be safe...
				buffer[bytes] = '\0';

				// parse
				parser_.feed(buffer);

				std::cout << "is parser done?: " << std::boolalpha << parser_.done() << "\n";
				std::cout << "Request: " << parser_.get() << "\n";
				if (parser_.done())
				{
					const Request& request_ = parser_.get();
					processor_.set(request_);
					//events_.modify(socket_.fd(), EPOLLOUT);
					work_state_ = Processing;		
				}
				break;
			}
		case Processing:
			{
				std::cout << "Processing\n";
				processor_.process();

				if (processor_.done())
				{
					response_ = processor_.response();
					work_state_ = Sending;		
				}
				break;
			}
		case Sending:
			{
				std::cout << "SENDING\n";
				response_.send(socket_.fd());

				if (response_.done())
				{
					std::cout << response_ << "\n";
					work_state_ = Done;		
					//@TODO cleanup de quê?
				}
				break;
			}
		default: 
			break;
	}
}

bool Connection::done() const
{
	return work_state_ == Done;
}

int Connection::fd() const
{
	return socket_.fd();
}
