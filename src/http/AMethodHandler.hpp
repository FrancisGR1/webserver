#ifndef AMETHODHANDLER_HPP
#define AMETHODHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "NewHttpResponse.hpp"

class AMethodHandler
{
	public:
		//type - POST, GET, DELETE
		virtual void process() = 0;
		virtual bool done() const = 0;
		virtual const NewHttpResponse& response() const = 0;
		virtual ~AMethodHandler() = 0;
};

#endif // AMETHODHANDLER_HPP
