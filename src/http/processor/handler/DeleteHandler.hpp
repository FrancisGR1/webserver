#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/response/Response.hpp"

class DeleteHandler : public IRequestHandler
{
  public:
    // construct/destruct
    DeleteHandler(const RequestContext& ctx);
    ~DeleteHandler();

    // IRequestHandler interface
    void process();
    bool done() const;
    const Response& response() const;
    std::vector<EventAction> give_events();

  private:
    const RequestContext& m_ctx;
    Response m_response;
    bool m_done;
};

#endif // DELETEHANDLER_HPP
