#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client
{
	private:
		int			client_fd_;
		std::string	request_;
		std::string	response_;

	public:
		Client(int socket);
		~Client();

		/* getters e setters */
		int			getClient_fd();
		void		setClient_fd(int socket);
		std::string	&getRequest();
		void		setRequest(const std::string &request);
		std::string	&getResponse();
		void		setResponse(const std::string &response);

		void		handle();
		void		readRequest();
};

#endif /* CLIENT_HPP */