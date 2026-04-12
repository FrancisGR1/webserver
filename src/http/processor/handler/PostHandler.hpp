#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "core/Path.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"

//@REFACTOR: mudar para colocar o nome do ficheiro se estiver o field "name";
// caso contrário dá um nome aleatório ao ficheiro -> tipo a timestamp atual ou uma chave random
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

    // upload file identifiers
    std::string m_upload_file; // name
    std::string m_upload_uri;  // location + name
    Path m_upload_path;        // relative path

    int m_fd;
    ssize_t m_offset;

    // utils
    std::string uri() const;
    std::string make_file_name() const;
};

#endif // POSTHANDLER_HPP
