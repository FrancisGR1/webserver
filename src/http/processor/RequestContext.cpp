#include "http/processor/RequestContext.hpp"
#include "core/contracts.hpp"
#include "http/processor/RequestConfig.hpp"
#include "server/Connection.hpp"

RequestContext::RequestContext(Connection* connection, const ServiceConfig& service)
    : m_connection(connection)
    , m_socket(connection->socket())
    , m_request_config(new RequestConfig(service))
{
}

const RequestConfig& RequestContext::config() const
{
    REQUIRE(m_request_config != NULL, "RequestContext: Tried to access request config Null Pointer!");

    return *m_request_config;
}

RequestConfig& RequestContext::config()
{
    REQUIRE(m_request_config != NULL, "RequestContext: Tried to access request config Null Pointer!");

    return *m_request_config;
}

Connection* RequestContext::connection() const
{
    return m_connection;
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
