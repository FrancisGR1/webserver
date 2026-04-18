#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/response/Response.hpp"

class DeleteHandler : public IRequestHandler
{
  public:
    DeleteHandler(const Request& request, const RequestContext& ctx);
    void process();
    bool done() const;
    const Response& response() const;
    ~DeleteHandler();

  private:
    const RequestContext& m_ctx;
    Response m_response;
    bool m_done;
    CgiHandler m_cgi;
};

#endif // DELETEHANDLER_HPP
