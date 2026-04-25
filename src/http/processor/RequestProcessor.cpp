#include "http/processor/RequestProcessor.hpp"
#include "core/Logger.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/processor/handler/DeleteHandler.hpp"
#include "http/processor/handler/ErrorHandler.hpp"
#include "http/processor/handler/GetHandler.hpp"
#include "http/processor/handler/PostHandler.hpp"
#include "http/response/ResponseError.hpp"

class Connection;

RequestProcessor::RequestProcessor(Connection* connection, const ServiceConfig& service)
    : m_state(RequestProcessor::Validating)
    , m_ctx(connection, service)
    , m_handler(NULL)
{
    Logger::trace("RequestProcessor: constructor");
}

RequestProcessor::~RequestProcessor()
{
    Logger::trace("RequestProcessor: destructor");
    delete m_handler;
};

std::string RequestProcessor::resolve_path(
    const std::string& req_path,
    const ServiceConfig& service,
    const LocationConfig*& location_ptr)
{
    bool is_dir = !req_path.empty() && req_path[req_path.size() - 1] == '/';

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
                throw ResponseError(StatusCode::BadRequest, "Target path tried to escape root");
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
    std::string location_str = cleaned_path;
    std::string resolved_path = cleaned_path;
    Logger::trace("RequestProcessor: search '%s'", location_str.c_str());
    while (true)
    {
        Logger::trace("RequestProcessor: Find: '%s'", location_str.c_str());
        if (utils::contains(service.locations, location_str))
        {
            Logger::trace("RequestProcessor: Found: '%s'", location_str.c_str());
            const std::string& root = service.locations.at(location_str).root_dir;
            resolved_path = utils::join_paths(root, cleaned_path);
            location_ptr = &service.locations.at(location_str);
            break;
        }

        // nothing to eat
        if (location_str == "/")
            break;

        // eat location_str
        size_t pos = location_str.find_last_of("/");
        location_str = (pos == 0) ? "/" : location_str.substr(0, pos);
    }
    Logger::debug("RequestProcessor: Resolved path to: '%s'", resolved_path.c_str());
    //@TODO: se não tem root da localização, então devemos dar root ou do service ou do webserver
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
                Logger::debug("RequestProcessor: state - Validating");
                if (m_request.bad_request())
                    throw ResponseError(m_request.status_code(), "Bad request status code", &m_ctx);

                m_state = Resolving;
            }
                // fall through

            case Resolving:
            {
                Logger::debug("RequestProcessor: state - Resolving");

                const ServiceConfig& service = m_ctx.config().service();

                // resolve
                const LocationConfig* matched_location = NULL;
                Path path = resolve_path(m_request.target_path(), service, matched_location);
                if (!path.exists)
                {
                    throw ResponseError(
                        StatusCode::NotFound, utils::fmt("'%s' path not found", path.raw.c_str()), &m_ctx);
                }

                // add to context
                m_ctx.config().set(path);
                if (matched_location)
                {
                    Logger::debug("RequestProcessor: Found location '%s'", matched_location->name.c_str());
                    m_ctx.config().set(*matched_location);
                }
                else
                {
                    Logger::debug("RequestProcessor: Didn't find location '%s'", path.raw.c_str());
                }
                m_state = Dispatching;
            }
                // fall through

            case Dispatching:
            {
                Logger::debug("RequestProcessor: state - Dispatching");

                if (m_ctx.config().is_cgi())
                    m_handler = new CgiHandler(m_request, m_ctx);
                else if (m_request.method() == "GET")
                    m_handler = new GetHandler(m_ctx);
                else if (m_request.method() == "POST")
                    m_handler = new PostHandler(m_request, m_ctx);
                else if (m_request.method() == "DELETE")
                    m_handler = new DeleteHandler(m_ctx);
                else
                    throw ResponseError(StatusCode::BadRequest, "Didn't find handler");
                m_state = Handling;
            }
                // fall through

            case Handling:
            {
                Logger::debug("RequestProcessor: state - Handling");
                m_handler->process();
                if (m_handler->done())
                {
                    m_state = Done;
                }
                break;
            }
            case Done:
            {
                Logger::debug("RequestProcessor: state - Done");
                break;
            }
        }
    }
    catch (const ResponseError& e)
    {
        Logger::error("RequestProcessor: %s", e.msg().c_str());
        delete m_handler;
        m_handler = new ErrorHandler(e);

        // Error handler does it in one go
        // so just process and finish
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
        throw std::logic_error("RequestProcessor: Tried to access handler Null Pointer!");
    }
    else
    {
        return m_handler->response();
    }
}

std::vector<EventAction> RequestProcessor::give_events()
{
    if (m_state >= Handling)
        return m_handler->give_events();
    return std::vector<EventAction>();
}

void RequestProcessor::set(const Request& request)
{
    m_request = request;
}

bool RequestProcessor::is_cgi() const
{
    return m_state >= Handling && m_ctx.config().is_cgi();
}
