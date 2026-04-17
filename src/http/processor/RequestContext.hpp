#ifndef REQUESTCONTEXT_HPP
#define REQUESTCONTEXT_HPP

#include "http/processor/RequestConfig.hpp"
#include "server/Socket.hpp"

class Connection;
// scoped to request processor
class RequestContext
{
  public:
    //@TODO: adicionar session manager e cookies?

    // construct/destruct
    RequestContext(Connection* connection, const ServiceConfig& service);
    ~RequestContext();

    // getters
    const RequestConfig& config() const;
    RequestConfig& config();
    Connection* connection() const;

  private:
    Connection* m_connection;
    const Socket& m_socket;
    RequestConfig* m_request_config;
};

std::ostream& operator<<(std::ostream& os, const RequestContext& ctx);

#endif // REQUESTCONTEXT_HPP
