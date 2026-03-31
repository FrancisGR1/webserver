#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "core/Path.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"

class PostHandler : public IRequestHandler
{
  public:
    PostHandler(const Request& request, const RequestContext& ctx);
    void process();
    bool done() const;
    const Response& response() const;
    ~PostHandler();

  private:
    const Request& m_request;
    const RequestContext& m_ctx;
    Response m_response;
    bool m_done;
    CgiHandler m_cgi;

    // upload
    Path m_upload;
    int m_fd;
    ssize_t m_offset;
    static unsigned long long m_uploaded_file_index;

    // utils
    ssize_t handle_upload(const std::string& body, size_t offset, int fd);
};

#endif // POSTHANDLER_HPP
