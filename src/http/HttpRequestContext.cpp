#include <stdexception>

#include "http/HttpRequestConfig.hpp"
#include "HttpRequestContext.hpp"

HttpRequestContext::HttpRequestContext()
	: m_request_config(NULL) {}

HttpRequestContext::HttpRequestContext(const ServiceConfig& service)
	: m_request_config(new HttpRequestConfig(service)) {}

const HttpRequestConfig& HttpRequestContext::config() const 
{ 
	if (m_request_config == NULL)
		throw std::runtime_error("HttpRequestContext: Tried to access request config Null Pointer!");
	return *m_request_config;
}

HttpRequestConfig& HttpRequestContext::config()
{ 
	if (m_request_config == NULL)
		throw std::runtime_error("HttpRequestContext: Tried to access request config Null Pointer!");
	return *m_request_config;
}

HttpRequestContext::~HttpRequestContext()
{
	delete m_request_config;
}
