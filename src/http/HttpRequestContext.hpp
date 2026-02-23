#ifndef REQUESTCONTEXT_HPP
# define REQUESTCONTEXT_HPP

#include "core/utils.hpp"
#include "config/ConfigTypes.hpp"
#include "http/HttpRequestConfig.hpp"

class HttpRequestContext
{
	public: 
		//@TODO: adicionar ligação, session manager e cookies
		HttpRequestContext();
		HttpRequestContext(const ServiceConfig& service);

		const HttpRequestConfig& config() const;
		HttpRequestConfig& config();
		~HttpRequestContext();

	private:
		HttpRequestConfig* m_request_config;
		//@TODO adicionar connection
};

#endif // REQUESTCONTEXT_HPP
