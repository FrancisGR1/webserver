#ifndef HTTPRESPONSEEXCEPTION_HPP
#define HTTPRESPONSEEXCEPTION_HPP

#include <string>

#include "config/types/LocationConfig.hpp"
#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"
#include "HttpRequestContext.hpp"

class ResponseError : public std::exception
{
	public:
		ResponseError(StatusCode::Code code, const std::string& msg, const HttpRequestContext* ctx = NULL);
		StatusCode::Code status() const;
		const std::string& msg() const;
		bool has_ctx() const;
		const HttpRequestContext& ctx() const;

		virtual ~ResponseError() throw();

	private:
		StatusCode::Code m_status;
		const std::string m_msg;
		const HttpRequestContext* m_ctx; // not owned, is nullable

		// illegal
		ResponseError();
};

#endif // HTTPRESPONSEEXCEPTION_HPP
