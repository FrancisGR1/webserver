#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"
#include "NewHttpResponse.hpp"
#include "ResponseError.hpp"
#include "HttpRequestContext.hpp"
#include "IRequestHandler.hpp"

class ErrorHandler : public IRequestHandler 
{
	public:
		ErrorHandler(const ResponseError& error);
		ErrorHandler(StatusCode::Code code);
		ErrorHandler(StatusCode::Code code, const HttpRequestContext& ctx);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;
		~ErrorHandler();

	private:
		NewHttpResponse m_response;
		StatusCode::Code m_code;
		const HttpRequestContext* m_ctx; // not owned, is nullable
		bool m_done;
};

#endif // ERRORHANDLER_HPP
