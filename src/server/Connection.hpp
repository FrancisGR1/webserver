#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "http/request/RequestParser.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"
#include "http/StatusCode.hpp"

#include <iostream>

class Connection
{
	private:
		enum State
		{
			Receiving = 0,
			Processing,
			Sending,
			Done
		};

		int					sock_;
		size_t				bytes_sent_;
		State state_;
		// http
		RequestParser		parser_;
		RequestProcessor	process_;
		Response			response_;

	public:
		Connection();
		Connection(int sock);
		~Connection();
		
		/* getters e setters */
		// int			getSock();
		// void		setSock(int sock);
		// HttpRequest	&getRequest();
		// void		setRequest(const std::string &request);
		// std::string	&getResponse();
		void process();
		void		setResponse(const std::string &response);
		bool		readRequest();

		bool		isReady() const;
		bool		sendResponse();
		
};

#endif /* CONNECTION_HPP */