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
    const std::vector<Path>& upload_paths() const;

  private:
    // dependencies
    const Request& m_request;
    const RequestContext& m_ctx;

    // state
    bool m_done;
    Response m_response;

    // upload tracking
    std::string m_upload_filename;    // name
    std::string m_upload_uri;         // location + name
    std::vector<Path> m_upload_paths; // full path
    int m_post_fd;                    // fd of posted file - owns
    ssize_t m_offset;                 // current write position
    size_t m_current_part;            // current multipart part

    // utils
    std::string make_uri() const;
    std::string make_file_name() const;
    void upload_request_body();
    void upload_multipart_body();
};

#endif // POSTHANDLER_HPP
