#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "NewHttpResponse.hpp"
#include "HttpRequestContext.hpp"
#include "AMethodHandler.hpp"

class DeleteHandler : public AMethodHandler 
{
	public:
		DeleteHandler(const HttpRequest& request, const HttpRequestContext& ctx);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;

		~DeleteHandler();

	private:
		NewHttpResponse m_response;
};

#endif // DELETEHANDLER_HPP
