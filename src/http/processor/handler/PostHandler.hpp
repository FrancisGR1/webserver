#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "core/Path.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"

class PostHandler : public IRequestHandler
{
  public:
    // construct/destruct
    PostHandler(const Request& request, const RequestContext& ctx);
    ~PostHandler();

    // IRequestHandler interface
    void process();
    bool done() const;
    const Response& response() const;
    std::vector<EventAction> give_events();

    // getters
    const Path& upload_path() const;

  private:
    // dependencies
    const Request& m_request;
    const RequestContext& m_ctx;

    // state
    bool m_done;
    Response m_response;

    // upload tracking
    std::string m_upload_filename; // name
    std::string m_upload_uri;      // location + name
    Path m_upload_path;            // full path
    int m_post_fd;                 // fd of posted file - owns
    ssize_t m_offset;              // current write position

    // utils
    std::string make_uri() const;
    std::string make_file_name() const;
};

#endif // POSTHANDLER_HPP
