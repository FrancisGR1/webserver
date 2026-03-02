#ifndef REQUESTPROCESSOR_HPP
# define REQUESTPROCESSOR_HPP

#include "config/types/ServiceConfig.hpp"
#include "http/request/Request.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/response/Response.hpp"
#include "server/EventManager.hpp"

class RequestProcessor : public IRequestHandler
{
	public:
		//@TODO adicionar informação da ligação
		RequestProcessor(const ServiceConfig& service, const EventManager& events);

		void process();
		bool done() const;
		const Response& response() const;
		void set(const Request& request);
		~RequestProcessor();

	private:
		enum State 
		{
			Validating = 0,
			Resolving,
			Dispatching,
			Handling,
			Done
		};

		const Request& m_request;
		State m_state;
		RequestContext m_ctx;
		IRequestHandler* m_handler;
};

#endif // REQUESTPROCESSOR_HPP
