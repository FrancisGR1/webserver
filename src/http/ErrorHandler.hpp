#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "config/ConfigTypes.hpp"
#include "NewHttpResponse.hpp"
#include "AMethodHandler.hpp"

class ErrorHandler : public AMethodHandler 
{
	public:
		ErrorHandler(StatusCode::Code code);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;
		~ErrorHandler();

	private:
		NewHttpResponse m_response;
};

#endif // ERRORHANDLER_HPP
