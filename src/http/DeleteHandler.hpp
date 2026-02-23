#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "NewHttpResponse.hpp"
#include "HttpRequestContext.hpp"
#include "IRequestHandler.hpp"
#include "CgiHandler.hpp"

class DeleteHandler : public IRequestHandler 
{
	public:
		DeleteHandler(const HttpRequest& request, const HttpRequestContext& ctx);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;
		~DeleteHandler();

	private:
		const HttpRequest& m_request; 
		const HttpRequestContext& m_ctx;
		NewHttpResponse m_response;
		bool m_done;
		CgiHandler m_cgi;
};

#endif // DELETEHANDLER_HPP
