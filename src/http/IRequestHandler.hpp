#ifndef AMETHODHANDLER_HPP
#define AMETHODHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "NewHttpResponse.hpp"

class IRequestHandler
{
	public:
		//type - Async model for request processor and method handlers
		virtual void process() = 0;
		virtual bool done() const = 0;
		virtual const NewHttpResponse& response() const = 0;
		virtual ~IRequestHandler() = 0;
};

#endif // AMETHODHANDLER_HPP
