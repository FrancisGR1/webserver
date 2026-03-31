#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "config/types/ServiceConfig.hpp"

class Socket
{
	public:
		Socket(int fd, const ServiceConfig& service);
		~Socket();

		int						fd() const;
		int close();
		const ServiceConfig&	service() const;

	private:
		int						fd_;
		const ServiceConfig&	service_;

		// illegal
		Socket(const Socket&);
		Socket& operator=(const Socket&);
};

#endif /* SOCKET_HPP */
