#ifndef REQUESTCONTEXT_HPP
# define REQUESTCONTEXT_HPP

#include "core/utils.hpp"
#include "config/ConfigTypes.hpp"
#include "http/HttpRequestConfig.hpp"

class HttpRequestContext
{
	public: 
		//@TODO: adicionar ligação, session manager e cookies
		HttpRequestContext(const HttpRequestConfig& request_config);
		const HttpRequestConfig& config() const;

	private:
		const HttpRequestConfig& m_request_config;
};

#endif // REQUESTCONTEXT_HPP
