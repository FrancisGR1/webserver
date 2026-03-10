#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "server/EventManager.hpp"
#include "server/Socket.hpp"
#include "http/request/RequestParser.hpp"
#include "http/processor/RequestProcessor.hpp"

class Connection
{
	public:
		Connection(Socket& conn_socket, EventManager& events);
		~Connection();
		
		void		work(const epoll_event& on_event);
		bool		done() const;
		int fd() const;
		
	private:

		enum State
		{
			Receiving = 0,
			Processing,
			Sending,
			Done
		};

		// saves current work state
		State					work_state_;

		// context
		Socket& socket_; // owns the socket
		const ServiceConfig& 	service_;

		// will add/remove/modify events
		EventManager&			events_;

		// http
		RequestParser			parser_;
		RequestProcessor		processor_;
		Response			response_;

		// illegal - no copying of connections is allowed
		// every connection must be a reference/pointer
		// to a connection from <<<ConnectionPool>>>
		Connection(const Connection&);
		Connection& operator=(const Connection&);
};

#endif /* CONNECTION_HPP */
