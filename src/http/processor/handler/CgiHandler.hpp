#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <map>
#include <string>

#include "core/Path.hpp"
#include "core/Timer.hpp"
#include "core/constants.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"
#include "server/EventAction.hpp"

// https://datatracker.ietf.org/doc/html/rfc3875
class CgiHandler : public IRequestHandler
{
  public:
    // construct/destruct
    CgiHandler(const Request& request, const RequestContext& ctx, Seconds timeout = constants::cgi_max_output);
    ~CgiHandler();

    // IRequestHandler interface
    void process();
    bool done() const;
    const Response& response() const;
    std::vector<EventAction> give_events();

  private:
    enum State
    {
        Error = -1,
        Init,
        Pipe,
        CookData,
        Done
    };

    // dependencies
    const Request& m_request;
    const RequestContext& m_ctx;
    const Path& m_script;

    // state
    State m_state;
    Response m_response;
    Timer m_timer;
    Seconds m_timeout;
    size_t m_failed_reads;
    std::vector<EventAction> m_events;

    // environment
    std::map<std::string, std::string> m_env;

    // pipe files
    int m_read_from_script[2];
    int m_write_to_script[2];

    // cgi data state
    std::string m_headers;
    std::string m_body_str;
    size_t m_total_reads;
    pid_t m_subprocess_id;
    size_t m_body_offset;

    // utils
    void start_timer();
    void start_subprocess();
    void setup_pipes();
    void read_from_pipe();
    void write_to_pipe();
    void cook_read_data();
    void register_action(EventAction::Action action, int fd);
    std::map<std::string, std::string> init_env();
    std::string to_uppercase_and_underscore(const std::string& str);
    void expect_has_time_left() const;
};

#endif // CGI_HANDLER_HPP
