#include "StatusCode.hpp"
#include "ResponseError.hpp"
#include "RequestProcessor.hpp"

RequestProcessor::RequestProcessor(const HttpRequest& request, const ServiceConfig& service)
	: m_request(request)
	, m_state(RequestProcessor::Validating)
	, m_context(service)
	, m_handler(NULL) {}

void RequestProcessor::process()
{
	try
	{
		switch (m_state)
		{
			// initial setup is all made in one go, hence the fall through
			// so validating -> resolving -> dispatching
			case Validating:
				{
					if (m_request.bad_request())
						throw ResponseError(m_request.status_code(), "Bad request status code");
					m_state = Resolving;
				}
			case Resolving:
				{
					const ServiceConfig& service = m_context.config().service();
					Path path = resolve_target(m_request.target_path(), service);
					if (!path.exists)
					{
						throw ResponseError(StatusCode::NotFound, utils::fmt("'%s' path not found", path.raw.c_str()));
					}
					m_context.config().set(path);

					if (utils::contains(service, path.raw))
					{
						const LocationConfig& location = service.locations.at(path.raw);
						m_context.config().set(location);
					}
					m_state = Dispatching;
				}
			case Dispatching:
				{
					if      (m_request.method() == "GET")	    m_handler = new GetHandler(m_request, m_context);
					else if (m_request.method() == "POST")      m_handler = new PostHandler(m_request, m_context);
					else if (m_request.method() == "DELETE")    m_handler = new DeleteHandler(m_request, m_context);
					else 					    m_handler = new ErrorHandler(StatusCode::InternalServerError, m_context);
					m_state = Handling;
				}

			case Handling:
				{
					m_handler->process();
					if (m_handler->done())
					{
						m_state = Done;
					}
					break;
				}
			case Done:
				{
					break;
				}
		}
	}
	catch (const ResponseError& e)
	{
		delete m_handler;
		m_handler = new ErrorHandler(e);
		//Error handler does it in one go
		//so just process and finish
		m_handler->process();
		m_state = Done;
	}
	//@QUESTION all caught throws here should be ResponseError right?
}

bool RequestProcessor::done() const
{
	return m_state == Done;
}

const NewHttpResponse& RequestProcessor::response() const
{
	if (m_handler == NULL)
	{
		throw std::runtime_error("RequestProcessor: Tried to access handler Null Pointer!");
	}
	else
	{
		return m_handler->response();
	}
}
RequestProcessor::~RequestProcessor() 
{
	delete m_handler;
};
