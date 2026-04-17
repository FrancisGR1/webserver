#include <stdexcept>

#include "http/processor/RequestConfig.hpp"
#include "http/processor/RequestContext.hpp"
#include "server/Connection.hpp"

RequestContext::RequestContext(Connection* connection, const ServiceConfig& service)
    : m_connection(connection)
    , m_socket(connection->socket())
    , m_request_config(new RequestConfig(service))
{
}

const RequestConfig& RequestContext::config() const
{
    if (m_request_config == NULL)
        throw std::logic_error("RequestContext: Tried to access request config Null Pointer!");
    return *m_request_config;
}

Connection* RequestContext::connection() const
{
    return m_connection;
}

RequestConfig& RequestContext::config()
{
    if (m_request_config == NULL)
        throw std::logic_error("RequestContext: Tried to access request config Null Pointer!");
    return *m_request_config;
}

RequestContext::~RequestContext()
{
    delete m_request_config;
}

std::ostream& operator<<(std::ostream& os, const RequestContext& ctx)
{
    os << ctx.config();
    return os;
}
