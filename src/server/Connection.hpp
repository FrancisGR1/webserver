#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <iostream>

class Connection
{
	private:
		int			sock_;
		std::string	request_;
		std::string	response_;

	public:
		Connection(int sock);
		~Connection();

		/* getters e setters */
		int			getSock();
		void		setSock(int sock);
		std::string	&getRequest();
		void		setRequest(const std::string &request);
		std::string	&getResponse();
		void		setResponse(const std::string &response);

		void		handle();
		void		readRequest();
};

#endif /* CONNECTION_HPP */