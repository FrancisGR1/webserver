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
		explicit ResponseError(StatusCode::Code code, const std::string& msg, const LocationConfig& lc);
		StatusCode::Code status() const;
		const LocationConfig& location() const;
		const std::string& msg() const;

		virtual ~ResponseError() throw();

	private:
		StatusCode::Code m_status;
		LocationConfig m_location; //@TODO isto deve ser um pointer
		std::string m_msg;

		// illegal
		ResponseError();
};

#endif // HTTPRESPONSEEXCEPTION_HPP
