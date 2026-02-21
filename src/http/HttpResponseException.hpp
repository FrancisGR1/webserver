#ifndef HTTPRESPONSEEXCEPTION_HPP
#define HTTPRESPONSEEXCEPTION_HPP

#include <string>

#include "config/types/LocationConfig.hpp"
#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"

class ResponseError : public std::exception
{
	public:
		explicit ResponseError(StatusCode::Code code, const std::string& msg);
		StatusCode::Code status() const;
		const std::string& msg() const;

		virtual ~ResponseError() throw();

	private:
		StatusCode::Code m_status;
		std::string m_msg;

		// illegal
		ResponseError();
};

#endif // HTTPRESPONSEEXCEPTION_HPP
