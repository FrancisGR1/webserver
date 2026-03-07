#ifndef REQUESTCONTEXT_HPP
# define REQUESTCONTEXT_HPP

#include "server/EventManager.hpp"
#include "server/Socket.hpp"
#include "http/processor/RequestConfig.hpp"

class RequestContext
{
	public: 
		//@TODO: adicionar ligação(?), session manager e cookies
		RequestContext();
		RequestContext(const Socket& conn_socket, EventManager& events, const ServiceConfig& service);

		const RequestConfig& config() const;
		RequestConfig& config();
		~RequestContext();

	private:
		const Socket& m_socket;
		EventManager& m_events;
		RequestConfig* m_request_config;
};

#endif // REQUESTCONTEXT_HPP
