#include <exception>

#include "http/processor/RequestConfig.hpp"
#include "http/processor/RequestContext.hpp"

RequestContext::RequestContext(const Socket& conn_socket, EventManager& events, const ServiceConfig& service)
	: m_socket(conn_socket)
	, m_events(events)
	, m_request_config(new RequestConfig(service)) 
	{
		(void)m_socket;
		(void)m_events;
	}

const RequestConfig& RequestContext::config() const 
{ 
	if (m_request_config == NULL)
		throw std::logic_error("RequestContext: Tried to access request config Null Pointer!");
	return *m_request_config;
}

RequestConfig& RequestContext::config()
{ 
	if (m_request_config == NULL)
		throw std::logic_error("RequestContext: Tried to access request config Null Pointer!");
	return *m_request_config;
}

RequestContext::~RequestContext()
{
	delete m_request_config;
}
