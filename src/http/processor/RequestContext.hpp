#ifndef REQUESTCONTEXT_HPP
# define REQUESTCONTEXT_HPP

#include "core/utils.hpp"
#include "config/ConfigTypes.hpp"
#include "http/processor/RequestConfig.hpp"

class RequestContext
{
	public: 
		//@TODO: adicionar ligação, session manager e cookies
		RequestContext();
		RequestContext(const ServiceConfig& service);

		const RequestConfig& config() const;
		RequestConfig& config();
		~RequestContext();

	private:
			//@TODO adicionar connection
		EventManager m_events;
		RequestConfig* m_request_config;
};

#endif // REQUESTCONTEXT_HPP
