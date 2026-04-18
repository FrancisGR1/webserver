#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "core/Path.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/response/Response.hpp"
#include "server/EventAction.hpp"

class GetHandler : public IRequestHandler
{
  public:
    // construct/destruct
    GetHandler(const Request& request, const RequestContext& ctx);
    ~GetHandler();

    // IRequestHandler interface
    void process();
    bool done() const;
    const Response& response() const;

  private:
    // dependencies
    const RequestContext& m_ctx;

    // state
    bool m_done;
    CgiHandler m_cgi;
    int m_get_fd; // fd of file to get - owns
    Response m_response;

    // utils
    void handle_index(Response& response, const Path& path);
    void handle_autoindex(Response& response, const Path& path);
    std::string make_autoindex(const Path& path);
    void handle_file(Response& response, const Path& path);
};

#endif // GETHANDLER_HPP
