#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/response/Response.hpp"
#include "server/EventAction.hpp"

class DeleteHandler : public IRequestHandler
{
  public:
    DeleteHandler(const Request& request, const RequestContext& ctx);
    void process();
    bool done() const;
    const Response& response() const;
    std::vector<EventAction> give_events();
    ~DeleteHandler();

  private:
    const RequestContext& m_ctx;
    Response m_response;
    bool m_done;
    CgiHandler m_cgi;
    std::vector<EventAction> m_events;
};

#endif // DELETEHANDLER_HPP
