#ifndef AMETHODHANDLER_HPP
#define AMETHODHANDLER_HPP

#include <vector>

#include "http/response/Response.hpp"
#include "server/EventAction.hpp"

class IRequestHandler
{
  public:
    // type - Async model for request processor and method handlers
    virtual void process() = 0;
    virtual bool done() const = 0;
    virtual const Response& response() const = 0;
    virtual std::vector<EventAction> give_events() = 0;
    virtual ~IRequestHandler() = 0;
};

#endif // AMETHODHANDLER_HPP
