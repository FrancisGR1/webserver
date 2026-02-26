#include "http/StatusCode.hpp"
#include "http/response/ResponseError.hpp"
#include "http/processor/RequestProcessor.hpp"
#include "http/processor/handler/GetHandler.hpp"
#include "http/processor/handler/PostHandler.hpp"
#include "http/processor/handler/DeleteHandler.hpp"
#include "http/processor/handler/ErrorHandler.hpp"

RequestProcessor::RequestProcessor(const ServiceConfig& service, const EventManager& events)
	: m_state(RequestProcessor::Validating)
	, m_ctx(service, events)
	, m_handler(NULL) {}

static std::string resolve_target(const std::string& req_path, const ServiceConfig& service)
{
	bool is_dir = !req_path.empty() &&
		req_path[req_path.size() - 1] == '/';

	// transform request path into clean path
	const std::vector<std::string>& segments = utils::str_split(req_path, "/");
	std::vector<std::string> legal_segments;
	for (size_t i = 0; i < segments.size(); ++i)
	{
		const std::string& s = segments[i];
		if (s.empty() || s == ".") 
			continue;
		if (s == "..")
		{
			if (!legal_segments.empty())
				legal_segments.pop_back();
			else // @NOTE: trying to escape root
			{
				throw ResponseError(
						StatusCode::BadRequest, 
						"Target path tried to escape root"
						);
			}
		}
		else
		{
			legal_segments.push_back(s);
		}
	}
	//@ASSUMPTION: target path always starts with '/'
	std::string cleaned_path = "/";
	for (size_t i = 0; i < legal_segments.size(); ++i)
	{
		cleaned_path += legal_segments[i];
		if (i + 1 < legal_segments.size() || is_dir)
			cleaned_path += "/";
	}

	// find the best matching location 
	std::string remainder;
	std::string location = cleaned_path;
	std::string resolved_path = cleaned_path;
	while (location != "/" && !location.empty())
	{

		size_t pos = location.find_last_of("/");
		location = location.substr(0, pos);
		remainder = cleaned_path.substr(pos);
		if (utils::contains(service.locations, location))
		{
			const std::string& root = service.locations.at(location).root_dir;
			resolved_path = utils::join_paths(root, remainder);
			break;
		}
	}

	return resolved_path;
}

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
						throw ResponseError(
								m_request.status_code(),
								"Bad request status code",
								&m_ctx
								);
					m_state = Resolving;
				}
			// fall through

			case Resolving:
				{
					const ServiceConfig& service = m_ctx.config().service();
					Path path = resolve_target(m_request.target_path(), service);
					if (!path.exists)
					{
						throw ResponseError(
								StatusCode::NotFound, 
								utils::fmt("'%s' path not found", path.raw.c_str()),
								&m_ctx
								);
					}
					m_ctx.config().set(path);

					if (utils::contains(service.locations, path.raw))
					{
						const LocationConfig& location = service.locations.at(path.raw);
						m_ctx.config().set(location);
					}
					m_state = Dispatching;
				}
			// fall through

			case Dispatching:
				{
					if      (m_request.method() == "GET")	    m_handler = new GetHandler(m_request, m_ctx);
					else if (m_request.method() == "POST")      m_handler = new PostHandler(m_request, m_ctx);
					else if (m_request.method() == "DELETE")    m_handler = new DeleteHandler(m_request, m_ctx);
					else 					    m_handler = new ErrorHandler(StatusCode::InternalServerError, m_ctx);
					m_state = Handling;
				}
			// fall through

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
}

bool RequestProcessor::done() const
{
	return m_state == Done;
}

const Response& RequestProcessor::response() const
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
