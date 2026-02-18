#ifndef STRINGBODY_HPP
# define STRINGBODY_HPP

#include <string>

#include "IBody.hpp"

class StringBody : public IBody
{
	public:
		StringBody(const std::string&  str);
		StringBody(const char* str);

		ssize_t send(int fd);
		size_t size() const;
		bool done() const;
		~StringBody();

	private:
		std::string m_body;
		size_t m_bytes_sent;
};

//cgi body
//in memory body (string)
//file body

#endif // STRINGBODY_HPP
