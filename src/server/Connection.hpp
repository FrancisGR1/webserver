#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "http/HttpRequestParser.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/StatusCode.hpp"

#include <iostream>

class Connection
{
	private:
		int					sock_;
		HttpRequestParser	parser_;
		std::string			response_;
		size_t				bytes_sent_;

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
		void		setResponse(const std::string &response);

		bool		readRequest();
		bool		isReady() const;
		bool		sendResponse();
		
};

#endif /* CONNECTION_HPP */