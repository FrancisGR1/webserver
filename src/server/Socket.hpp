#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "config/parser/ConfigParser.hpp"

class Socket
{
	private:
		int						fd_;
		const ServiceConfig&	service_;

	public:
		Socket(int fd, const ServiceConfig& service);
		~Socket();

		int						getFd() const;
		const ServiceConfig&	getService() const;

};

#endif /* SOCKET_HPP */