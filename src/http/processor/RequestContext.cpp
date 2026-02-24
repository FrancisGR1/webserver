#include <exception>

#include "http/processor/RequestConfig.hpp"
#include "http/processor/RequestContext.hpp"

RequestContext::RequestContext()
	: m_request_config(NULL) {}

RequestContext::RequestContext(const ServiceConfig& service)
	: m_request_config(new RequestConfig(service)) {}

const RequestConfig& RequestContext::config() const 
{ 
	if (m_request_config == NULL)
		throw std::runtime_error("RequestContext: Tried to access request config Null Pointer!");
	return *m_request_config;
}

RequestConfig& RequestContext::config()
{ 
	if (m_request_config == NULL)
		throw std::runtime_error("RequestContext: Tried to access request config Null Pointer!");
	return *m_request_config;
}

RequestContext::~RequestContext()
{
	delete m_request_config;
}
