#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "core/Path.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/response/Response.hpp"

class GetHandler : public IRequestHandler
{
  public:
    // construct/destruct
    GetHandler(const RequestContext& ctx);
    ~GetHandler();

    // IRequestHandler interface
    void process();
    bool done() const;
    const Response& response() const;
    std::vector<EventAction> give_events();

  private:
    // dependencies
    const RequestContext& m_ctx;

    // state
    bool m_done;
    int m_get_fd; // fd of file to get - owns
    Response m_response;

    // utils
    void handle_index(Response& response, const Path& path);
    void handle_autoindex(Response& response, const Path& path);
    std::string make_autoindex(const Path& path);
    void handle_file(Response& response, const Path& path);
};

#endif // GETHANDLER_HPP
