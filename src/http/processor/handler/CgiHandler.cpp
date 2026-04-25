#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "core/Path.hpp"
#include "core/constants.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "http/http_utils.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/request/Request.hpp"
#include "http/response/Response.hpp"
#include "http/response/ResponseError.hpp"
#include "server/Connection.hpp"

CgiHandler::CgiHandler(const Request& request, const RequestContext& ctx, Seconds timeout)
    : m_request(request)
    , m_ctx(ctx)
    , m_script(ctx.config().path())
    , m_state(Init)
    , m_response(StatusCode::Ok) //@NOTE: rfc says response is ok by default
    , m_timeout(timeout)
    , m_failed_reads(0)
    , m_env(init_env())
    , m_total_reads(0)
    , m_body_offset(0)
{
    Logger::trace("%s: constructor", constants::cgi);

    m_read_from_script[0] = -1;
    m_read_from_script[1] = -1;
    m_write_to_script[0] = -1;
    m_write_to_script[1] = -1;
}

CgiHandler::~CgiHandler()
{
    Logger::trace("%s: destructor", constants::cgi);
    if (m_read_from_script[0] != -1)
        ::close(m_read_from_script[0]);
    if (m_write_to_script[1] != -1)
        ::close(m_write_to_script[1]);
}

void CgiHandler::process()
{
    expect_has_time_left();

    switch (m_state)
    {
        //@TODO code in fast path for short cgi scripts
        case Init:
        {
            if (!m_ctx.config().allows_method(m_request.method()))
                http_utils::throw_method_not_allowed(m_request.method(), m_ctx);

            start_timer();
            setup_pipes();
            start_subprocess();

            m_state = Pipe;

            break;
        }
        //@TODO get current event to decide if you should write or read
        case Pipe:
        {
            write_to_pipe();
            read_from_pipe();
            break;
        }

        case Res:
        {
            make_response();

            break;
        }
        default:;
    }
}

bool CgiHandler::done() const
{
    return m_state == Done;
}

const Response& CgiHandler::response() const
{
    return m_response;
}

std::vector<EventAction> CgiHandler::give_events()
{
    std::vector<EventAction> result;
    result.swap(m_events);

    Logger::trace("%s: give '%zu' events", constants::cgi, result.size());

    return result;
}

void CgiHandler::start_timer()
{
    Logger::trace("%s: start timer", constants::cgi);
    m_timer.set(m_timeout);
    m_timer.start();
}

void CgiHandler::setup_pipes()
{
    if (::pipe(m_read_from_script) == -1)
    {
        throw ResponseError(
            StatusCode::InternalServerError, "CgiHandler: pipe() call to read from script failed!", &m_ctx);
    }
    fcntl(m_read_from_script[0], F_SETFL, O_NONBLOCK);

    // open stdin pipe if there is a body
    if (m_request.body().size() > 0)
    {
        if (::pipe(m_write_to_script) == -1)
        {
            close(m_read_from_script[0]);
            close(m_read_from_script[1]);

            throw ResponseError(
                StatusCode::InternalServerError, "CgiHandler: pipe() call to write to script failed!", &m_ctx);
        }
        fcntl(m_write_to_script[1], F_SETFL, O_NONBLOCK);
    }
}

void CgiHandler::start_subprocess()
{
    Logger::trace("%s: fork and execute", constants::cgi);

    // fork
    pid_t id = ::fork();
    if (id == -1)
        throw ResponseError(StatusCode::InternalServerError, "CgiHandler: fork() call failed!", &m_ctx);

    if (id == 0) // subprocess
    {
        // build argv
        Path interpreter = m_ctx.config().cgi_interpreter();
        Path script = m_ctx.config().path();
        char* argv[] = {const_cast<char*>(interpreter.c_str()), const_cast<char*>(script.cgi_name.c_str()), NULL};
        Logger::trace("%s: Subprocess: argv: '%s' '%s'", constants::cgi, interpreter.c_str(), script.cgi_name.c_str());

        if (::chdir(script.cgi_dir.c_str()) == -1)
        {
            Logger::error("%s: Subprocess: chdir() call failed. errno: '%s'", constants::cgi, ::strerror(errno));
            std::abort();
        }

        // build env
        std::vector<std::string> env_strings;
        std::vector<char*> envp;
        for (std::map<std::string, std::string>::const_iterator it = m_env.begin(); it != m_env.end(); ++it)
        {
            env_strings.push_back(it->first + "=" + it->second);
        }
        for (size_t i = 0; i < env_strings.size(); ++i)
            envp.push_back(const_cast<char*>(env_strings[i].c_str()));
        envp.push_back(NULL);
        Logger::trace("%s: Subprocess: env was built", constants::cgi);

        // flush logs before overriding stdout
        Logger::flush();

        // write to pipe
        dup2(m_read_from_script[1], STDOUT_FILENO);
        // dup2(m_read_from_script[1], STDERR_FILENO);
        if (m_write_to_script[0] != -1)
            dup2(m_write_to_script[0], STDIN_FILENO);

        // close pipes
        ::close(m_read_from_script[1]);
        ::close(m_read_from_script[0]);

        if (m_write_to_script[0] != -1)
        {
            ::close(m_write_to_script[1]);
            ::close(m_write_to_script[0]);
        }

        // execute
        ::execve(argv[0], argv, &envp[0]);
        Logger::error("%s: Subprocess: execve() gone wrong! Aborting!", constants::cgi);

        // on error, exit without cleanup
        // @TODO @QUESTION: isto chama os destrutores?
        std::abort();
    }
    else // main process
    {
        // save subprocess id
        m_subprocess_id = id;

        // close unused pipe ends
        ::close(m_read_from_script[1]);
        if (m_write_to_script[0] != -1)
            ::close(m_write_to_script[0]);

        // add events
        register_action(EventAction::WantRead, m_read_from_script[0]);
        if (m_write_to_script[1] != -1)
            register_action(EventAction::WantWrite, m_write_to_script[1]);
    }
}

void CgiHandler::read_from_pipe()
{
    // read pipe to buffer
    char buffer[constants::pipe_buffer_capacity + 1];
    ssize_t bytes = read(m_read_from_script[0], buffer, constants::pipe_buffer_capacity);
    Logger::trace("%s: read %ld from pipe", constants::cgi, bytes);

    if (bytes > 0) // read data
    {
        buffer[bytes] = '\0';

        m_total_reads += bytes;
        Logger::trace("%s: total bytes read: %zu", constants::cgi, m_total_reads);
        Logger::trace("%s: read: '%s'", constants::cgi, buffer);

        if (m_total_reads > constants::cgi_max_output)
        {
            throw ResponseError(
                StatusCode::BadGateway,
                utils::fmt(
                    "%s: output (%zu) exceeded max capacity (%zu)",
                    constants::cgi,
                    m_total_reads,
                    constants::cgi_max_output),
                &m_ctx);
        }

        m_headers += buffer;

        size_t body_start = m_headers.find(constants::crlfcrlf);
        if (body_start != std::string::npos)
        {
            // separate body and headers
            m_body_str = m_headers.substr(body_start + 4); // 4 = crlfcrlf size
            m_headers = m_headers.substr(0, body_start);

            Logger::trace("CgiHandler:\nheaders: '%s';\nbody:    '%s';", m_headers.c_str(), m_body_str.c_str());

            register_action(EventAction::WantClose, m_read_from_script[0]);

            // body might have not been totally read
            // but it's ok, Response can handle split bodies
            // (1st part in string and 2nd part in file)
            // so go to next state regardless
            make_response();
        }
    }
    else if (bytes == 0) // EOF
    {

        int status = 0;
        int res = waitpid(m_subprocess_id, &status, 0);
        if (res == -1)
        {
            throw ResponseError(
                StatusCode::BadGateway,
                utils::fmt("%s: waitpid failed. errno: '%s'", constants::cgi, ::strerror(errno)),
                &m_ctx);
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            throw ResponseError(
                StatusCode::BadGateway,
                utils::fmt("%s: subprocess exited with %d code", constants::cgi, WEXITSTATUS(status)),
                &m_ctx);
        }

        register_action(EventAction::WantClose, m_read_from_script[0]);
        if (m_write_to_script[1] != -1)
            register_action(EventAction::WantClose, m_write_to_script[1]);

        m_read_from_script[0] = -1;
        m_write_to_script[1] = -1;

        m_state = Res;
    }
    else // nothing was read
    {
        int err = errno;
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            Logger::warn("%s: read error: '%s'", constants::cgi, ::strerror(err));
        else
            Logger::debug("%s: pipe: '%s'", constants::cgi, ::strerror(err));
    }
}

void CgiHandler::write_to_pipe()
{
    // is done
    if (m_write_to_script[1] == -1)
        return;

    const std::string& body = m_request.body();

    if (body.empty())
    {
        register_action(EventAction::WantClose, m_write_to_script[1]);
        ::close(m_write_to_script[1]);
        m_write_to_script[1] = -1;
        return;
    }

    // write as much as possible in one go
    ssize_t bytes = ::write(m_write_to_script[1], body.c_str() + m_body_offset, body.size() - m_body_offset);

    if (bytes > 0)
    {
        m_body_offset += bytes;
        Logger::trace("%s: wrote %ld to pipe (total %zu/%zu)", constants::cgi, bytes, m_body_offset, body.size());
    }
    else
    {
        Logger::warn("%s: wrote nothing to pipe! errno: '%s'", constants::cgi, ::strerror(errno));
    }

    // close
    if (m_body_offset >= body.size())
    {
        ::close(m_write_to_script[1]);
        m_write_to_script[1] = -1;
        Logger::trace("%s: finished writing body to pipe", constants::cgi);
    }
}

// transform the data read from the pipe into a Http Response
void CgiHandler::make_response()
{
    Logger::trace("%s: transform headers to a string", constants::cgi);

    // https://www.rfc-editor.org/rfc/rfc3875#section-6
    // validate and set headers
    std::vector<std::string> lines = utils::str_split(m_headers, constants::crlf);
    for (size_t i = 0; i < lines.size(); ++i)
    {
        const std::string& line = lines[i];
        size_t colon_pos = line.find(":");
        if (colon_pos != std::string::npos)
        {
            // key
            std::string key = line.substr(0, colon_pos);
            utils::str_trim_sides(key, " \t");

            // value
            std::string value = line.substr(colon_pos + 1);
            utils::str_trim_sides(value, " \t");

            // status code
            if (key == "Status")
            {
                StatusCode::Code code = static_cast<StatusCode::Code>(std::atoi(value.c_str()));
                if (StatusCode::exists(code))
                    m_response.set_status(code);
                else
                    throw ResponseError(
                        StatusCode::BadGateway,
                        utils::fmt("%s: Code doesn't exist: %zu", constants::cgi, code),
                        &m_ctx);
            }

            m_response.set_header(key, value);
        }
        else
        {
            throw ResponseError(StatusCode::BadGateway, "CgiHandler: Bad Header", &m_ctx);
        }
    }

    Logger::trace("%s: headers:'%s'", constants::cgi, m_headers.c_str());
    Logger::trace("%s: body string: '%s'", constants::cgi, m_body_str.c_str());
    Logger::trace("%s: body fd: '%d'", constants::cgi, m_read_from_script[0]);

    m_response.set_body_as_str(m_body_str);
    if (m_read_from_script[0] != -1)
    {
        int body_fd = ::dup(m_read_from_script[0]);
        m_response.set_body_as_fd(body_fd); // rest of body is read directly to socket from pipe
    }

    // finish
    m_timer.stop();
    m_state = Done;
}

void CgiHandler::register_action(EventAction::Action action, int fd)
{
    EventAction event(action, EventAction::Pipe, fd, m_ctx.connection());
    m_events.push_back(event);

    Logger::trace("%s: register event: '%s'", constants::cgi, event.str().c_str());
}

std::map<std::string, std::string> CgiHandler::init_env()
{
    std::map<std::string, std::string> env;

    // default headers
    env["AUTH_TYPE"] = "";
    env["CONTENT_LENGTH"] = "";
    env["CONTENT_TYPE"] = m_script.mime;
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["PATH_INFO"] = m_script.cgi_info;
    env["PATH_TRANSLATED"] = "";
    env["QUERY_STRING"] = m_request.target_query();
    env["REMOTE_ADDR"] = "";
    env["REMOTE_HOST"] = "";
    env["REMOTE_IDENT"] = "";
    env["REMOTE_USER"] = "";
    env["REQUEST_METHOD"] = m_request.method();
    env["SCRIPT_NAME"] = m_script.cgi_name;
    env["SERVER_NAME"] = m_ctx.config().service().server_name;
    //@NOTE connection is never null, except in the tests
    env["SERVER_ADDR"] = m_ctx.connection() ? m_ctx.connection()->socket().listener().host : "";
    env["SERVER_PORT"] = m_ctx.connection() ? m_ctx.connection()->socket().listener().port : "";
    env["SERVER_PROTOCOL"] = constants::server_http_version;
    env["SERVER_SOFTWARE"] = constants::server_name;

    // make http headers CGI compliant
    for (std::map<std::string, std::string>::const_iterator it = m_request.headers().begin();
         it != m_request.headers().end();
         ++it)
    {
        std::string key = "HTTP_" + to_uppercase_and_underscore(it->first);
        env[key] = it->second;
    }
    return env;
}

std::string CgiHandler::to_uppercase_and_underscore(const std::string& str)
{
    std::string result;
    result.resize(str.size());

    for (size_t i = 0; i < str.size(); ++i)
    {
        char c = str[i] == '-' ? '_' : std::toupper(str[i]);

        result[i] = c;
    }
    return result;
}

void CgiHandler::expect_has_time_left() const
{
    if (m_timer.expired())
    {
        ::kill(m_subprocess_id, SIGKILL);
        ::waitpid(m_subprocess_id, NULL, 0); // reap it
        http_utils::throw_gateway_timeout(m_script.raw, m_timeout, m_ctx);
    }
}
