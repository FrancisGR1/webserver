#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include "http/StatusCode.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/response/Response.hpp"
#include "http/response/ResponseError.hpp"

class ErrorHandler : public IRequestHandler
{
  public:
    // construct/destruct
    ErrorHandler(const ResponseError& error);
    ErrorHandler(StatusCode::Code code);
    ErrorHandler(StatusCode::Code code, const RequestContext& ctx);
    ~ErrorHandler();

    // api
    void process();
    bool done() const;
    const Response& response() const;
    std::vector<EventAction> give_events();
    static std::string make_default_body(StatusCode::Code code);

  private:
    Response m_response;
    StatusCode::Code m_code;
    const RequestContext* m_ctx; // not owned, is nullable
    bool m_done;
    std::vector<EventAction> m_events;
};

#endif // ERRORHANDLER_HPP
