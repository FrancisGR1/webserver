#include "http/request/Request.hpp"
#include "http/response/processor/RequestProcessor.hpp"
#include "core/constants.hpp"
#include "Connection.hpp"
#include "Webserver.hpp"

Connection::Connection(const Socket& conn_socket, EventManager& events) 
	, work_state_(Receiving)
	, socket_(conn_socket)
	, service_(conn_socket.getService())
	, events_(events)
	, processor_(conn_socket, events) {}


Connection::~Connection() {}

void Connection::work()
{
	switch(work_state_)
	{
		case Receiving:
			{
				// read client info to buffer
				char buffer[constants::read_chunk_size];
				ssize_t bytes = recv(socket_.getFd(), buffer, sizeof(buffer), 0);
				if (bytes <= 0)
				{
					//@TODO
					break;
				}

				// parser works on raw strings but just to be safe...
				buffer[bytes] = '\0';

				// parse
				parser_.feed(buffer);

				if (parser_.done())
				{
					const Request& request_ = parser_.get();
					processor_.set(request_);
					work_state_ = Processing;		
				}
				break;
			}
		case Processing:
			{
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
				response_.send(socket_.getFd());

				if (response_.done())
				{
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
