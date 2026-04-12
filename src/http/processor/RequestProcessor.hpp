#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "config/types/ServiceConfig.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"
#include "server/EventManager.hpp"

class RequestProcessor : public IRequestHandler
{
  public:
    //@TODO adicionar informação da ligação
    RequestProcessor(const Socket& conn_socket);
    ~RequestProcessor();

    void process();
    bool done() const;
    const Response& response() const;
    std::vector<EventAction> give_events();
    void set(const Request& request);

    static std::string resolve_path(
        const std::string& req_path,
        const ServiceConfig& service,
        const LocationConfig*& location_ptr);

  private:
    enum State
    {
        Validating = 0,
        Resolving,
        Dispatching,
        Handling,
        Done
    };

    Request m_request;
    State m_state;
    RequestContext m_ctx;
    std::vector<EventAction> m_events;
    IRequestHandler* m_handler;
};

#endif // REQUESTPROCESSOR_HPP
