#ifndef IBODY_HPP
# define IBODY_HPP

#include <cstddef>
#include <sys/types.h>

class IBody
{
	public:
		virtual ssize_t send(int fd) = 0;
		virtual size_t size() const = 0;
		virtual bool done() const = 0;
		virtual ~IBody() = 0;

	private:
};

#endif // IBODY_HPP
