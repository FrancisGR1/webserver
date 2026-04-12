#ifndef REQUESTCONTEXT_HPP
#define REQUESTCONTEXT_HPP

#include "http/processor/RequestConfig.hpp"
#include "server/Socket.hpp"

// scoped to request processor
class RequestContext
{
  public:
    //@TODO: adicionar session manager e cookies?
    RequestContext(const Socket& conn_socket, const ServiceConfig& service);

    const RequestConfig& config() const;
    RequestConfig& config();
    ~RequestContext();

  private:
    const Socket& m_socket;
    RequestConfig* m_request_config;
};

std::ostream& operator<<(std::ostream& os, const RequestContext& ctx);

#endif // REQUESTCONTEXT_HPP
