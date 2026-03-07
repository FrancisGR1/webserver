#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "server/EventManager.hpp"
#include "server/Socket.hpp"
#include "http/request/RequestParser.hpp"
#include "http/processor/RequestProcessor.hpp"

class Connection
{
	public:
		Connection(const Socket& conn_socket, EventManager& events);
		~Connection();
		
		void		work();
		bool		done() const;
		
	private:

		enum State
		{
			Receiving = 0,
			Processing,
			Sending,
			Done
		};
		// saves work state
		State					work_state_;

		// context
		const Socket& socket_;
		const ServiceConfig& 	service_;

		// will add/remove/modify events
		EventManager&			events_;

		// http
		RequestParser			parser_;
		RequestProcessor		processor_;
		Response			response_;

};

#endif /* CONNECTION_HPP */
