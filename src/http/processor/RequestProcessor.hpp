#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "config/types/ServiceConfig.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"
#include "server/EventAction.hpp"

class Connection;

class RequestProcessor : public IRequestHandler
{
  public:
    // construct/destruct
    RequestProcessor(Connection* connection, const ServiceConfig& service);
    ~RequestProcessor();

    // IRequestHandler interface
    void process();
    bool done() const;
    const Response& response() const;
    std::vector<EventAction> give_events();

    // setters
    void set(const Request& request);

    // getters
    bool is_cgi() const;

    // utils
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
    IRequestHandler* m_handler;
};

#endif // REQUESTPROCESSOR_HPP
