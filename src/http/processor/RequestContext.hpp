#ifndef REQUESTCONTEXT_HPP
# define REQUESTCONTEXT_HPP

#include "server/EventManager.hpp"
#include "server/Socket.hpp"
#include "http/processor/RequestConfig.hpp"

// scoped to request processor
class RequestContext
{
	public: 
		//@TODO: adicionar session manager e cookies?
		RequestContext(const Socket& conn_socket, EventManager& events, const ServiceConfig& service);

		//RequestContext(const Socket& conn_socket, EventManager& events, const RequestConfig& request_config);
		const RequestConfig& config() const;
		RequestConfig& config();
		~RequestContext();

	private:
		const Socket& m_socket;
		EventManager& m_events;
		RequestConfig* m_request_config;
};

#endif // REQUESTCONTEXT_HPP
