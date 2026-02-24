#ifndef HTTPRESPONSEEXCEPTION_HPP
#define HTTPRESPONSEEXCEPTION_HPP

#include <string>

#include "config/types/LocationConfig.hpp"
#include "config/ConfigTypes.hpp"
#include "http/StatusCode.hpp"
#include "http/processor/RequestContext.hpp"

class ResponseError : public std::exception
{
	public:
		ResponseError(StatusCode::Code code, const std::string& msg, const RequestContext* ctx = NULL);
		StatusCode::Code status() const;
		const std::string& msg() const;
		bool has_ctx() const;
		const RequestContext& ctx() const;

		virtual ~ResponseError() throw();

	private:
		StatusCode::Code m_status;
		const std::string m_msg;
		const RequestContext* m_ctx; // not owned, is nullable

		// illegal
		ResponseError();
};

#endif // HTTPRESPONSEEXCEPTION_HPP
