#include "http/HttpRequestConfig.hpp"
#include "HttpRequestContext.hpp"

HttpRequestContext::HttpRequestContext(const HttpRequestConfig& request_config)
	: m_request_config(request_config)
{
}

const HttpRequestConfig& HttpRequestContext::config() const { return m_request_config; }
