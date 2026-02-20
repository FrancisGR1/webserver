#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "NewHttpResponse.hpp"
#include "HttpRequestContext.hpp"
#include "AMethodHandler.hpp"

class GetHandler : public AMethodHandler 
{
	public:
		GetHandler(const HttpRequest& request, const HttpRequestContext& ctx);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;
		~GetHandler();

	private:
		NewHttpResponse m_response;
};

#endif // GETHANDLER_HPP
