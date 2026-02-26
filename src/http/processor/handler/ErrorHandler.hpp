#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "http/StatusCode.hpp"
#include "http/response/Response.hpp"
#include "http/response/ResponseError.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"

class ErrorHandler : public IRequestHandler 
{
	public:
		ErrorHandler(const ResponseError& error);
		ErrorHandler(StatusCode::Code code);
		ErrorHandler(StatusCode::Code code, const RequestContext& ctx);
		void process();
		bool done() const;
		const Response& response() const;
		~ErrorHandler();

	private:
		Response m_response;
		StatusCode::Code m_code;
		const RequestContext* m_ctx; // not owned, is nullable
		bool m_done;
};

#endif // ERRORHANDLER_HPP
