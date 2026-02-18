#ifndef HTTPRESPONSEEXCEPTION_HPP
#define HTTPRESPONSEEXCEPTION_HPP

#include <string>

#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"

class HttpResponseException : public std::exception
{
	public:
		explicit HttpResponseException(StatusCode::Code code, const std::string& msg);
		explicit HttpResponseException(StatusCode::Code code, const std::string& msg, const LocationConfig& lc);
		StatusCode::Code status() const;
		const LocationConfig& location() const;
		const std::string& msg() const;

		virtual ~HttpResponseException() throw();

	private:
		StatusCode::Code m_status;
		LocationConfig m_location; //@TODO isto deve ser um pointer
		std::string m_msg;

		// illegal
		HttpResponseException();
};

#endif // HTTPRESPONSEEXCEPTION_HPP
