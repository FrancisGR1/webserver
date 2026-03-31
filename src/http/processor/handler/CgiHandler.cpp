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

CgiHandler::CgiHandler(const Request& request, const RequestContext& ctx, Seconds timeout)
    : m_request(request)
    , m_ctx(ctx)
    , m_script(ctx.config().path())
    , m_response(StatusCode::Ok) //@ASSUMPTION: response is ok by default
    , m_timeout(timeout)
    , m_failed_reads(0)
    , m_env(init_env())
    , m_total_read(0)
    , m_state(StartTimer)
{
    Logger::trace("CgiHandler: constructor");
}

void CgiHandler::process()
{
    expect_has_time_left();

    switch (m_state)
    {
        case StartTimer:
        {
            Logger::trace("CgiHandler: start timer");
            m_timer.set(m_timeout);
            m_timer.start();
            m_state = ForkExec;
        }
            // fall through

        case ForkExec:
        {
            Logger::trace("CgiHandler: fork and execute");

            // pipe
            if (::pipe(m_fd) == -1)
                throw ResponseError(StatusCode::InternalServerError, "CgiHandler: pipe() call failed!", &m_ctx);

            // fork
            pid_t id = ::fork();
            if (id == -1)
                throw ResponseError(StatusCode::InternalServerError, "CgiHandler: fork() call failed!", &m_ctx);

            if (id == 0) // subprocess
            {
                // build argv
                Path interpreter = m_ctx.config().cgi_interpreter();
                char* interpreter_raw_path = const_cast<char*>(interpreter.raw.c_str());
                char* script_raw_path = const_cast<char*>(m_ctx.config().path().raw.c_str());
                char* argv[] = {interpreter_raw_path, script_raw_path, NULL};
                Logger::trace("CgiHandler: Subprocess: argv: '%s' '%s'", interpreter_raw_path, script_raw_path);

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
                Logger::trace("CgiHandler: Subprocess: env was built");

                // flush logs before overriding stdout
                Logger::flush();

                // write to pipe
                dup2(m_fd[1], STDOUT_FILENO);
                dup2(m_fd[1], STDERR_FILENO);

                // close pipe
                close(m_fd[1]);
                close(m_fd[0]);

                // execute
                execve(argv[0], argv, &envp[0]);
                Logger::error("CgiHandler: Subprocess: execve() gone wrong! Aborting!");

                // on error, exit without cleanup
                std::abort();
            }
            else // main process
            {
                // save subprocess id
                m_subprocess_id = id;

                // manage resources
                //@TODO: add fd [0] to events pool
                close(m_fd[1]);
                fcntl(m_fd[0], F_SETFL, O_NONBLOCK);

                // check subprocess status (in case of fast fail)
                // @TODO: é melhor isto ir para read pipe não?
                m_state = ReadPipe;
                break;
            }
        }
        case ReadPipe:
        {
            int status;
            waitpid(m_subprocess_id, &status, WNOHANG); // don't wait for subprocess
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            {
                throw ResponseError(
                    StatusCode::BadGateway, utils::fmt("CgiHandler: subprocess exited with %d code", status), &m_ctx);
            }

            Logger::trace("CgiHandler: read pipe");

            // read pipe to buffer
            char buffer[constants::read_pipe_chunk_size + 1];
            ssize_t bytes = read(m_fd[0], buffer, constants::read_pipe_chunk_size);
            Logger::trace("CgiHandler: read %ld from pipe", bytes);

            if (bytes > 0) // read data
            {
                buffer[bytes] = '\0';

                m_total_read += bytes;
                Logger::trace("CgiHandler: total bytes read: %zu", m_total_read);
                if (m_total_read > constants::cgi_max_output)
                {
                    throw ResponseError(
                        StatusCode::BadGateway,
                        utils::fmt(
                            "CgiHandler: output (%zu) exceeded max capacity (%zu)",
                            m_total_read,
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

                    // body might have not been totally read
                    // but it's ok, Response can handle split bodies
                    // (1st part in string and 2nd part in file)
                    // so go to next state regardless
                    m_state = CookData;
                }
            }
            else if (bytes == 0) // EOF
            {
                ::close(m_fd[0]);
                m_fd[0] = -1;
                m_state = CookData;
            }
            else // nothing was read
            {
                ++m_failed_reads;
                if (m_failed_reads == constants::cgi_max_failed_reads)
                {
                    ::kill(m_subprocess_id, SIGKILL);
                    throw ResponseError(
                        StatusCode::BadGateway,
                        utils::fmt("CgiHandler: Failed to read the pipe %zu times!", m_failed_reads),
                        &m_ctx);
                };
                Logger::warn("CgiHandler: Failed to read the pipe %zu times!", m_failed_reads);
            }
            break;
        }

        case CookData:
        {
            Logger::trace("CgiHandler: transform headers to a string");

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
                                utils::fmt("CgiHandler: Code doesn't exist: %zu", code),
                                &m_ctx);
                    }

                    m_response.set_header(key, value);
                }
                else
                {
                    throw ResponseError(StatusCode::BadGateway, "CgiHandler: Bad Header", &m_ctx);
                }
            }

            Logger::trace("CgiHandler: headers:'%s'", m_headers.c_str());
            Logger::trace("CgiHandler: body string: '%s'", m_body_str.c_str());
            Logger::trace("CgiHandler: body fd: '%d'", m_fd[0]);

            m_response.set_body_as_str(m_body_str);
            if (m_fd[0] != -1)
                m_response.set_body_as_fd(m_fd[0]); // rest of body is read directly to socket from pipe

            // finish
            m_timer.stop();
            m_state = Done;
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

CgiHandler::~CgiHandler()
{
    Logger::trace("CgiHandler: destructor");
}

std::map<std::string, std::string> CgiHandler::init_env()
{
    std::map<std::string, std::string> env;

    // default headers
    env["AUTH_TYPE"] = "";
    env["CONTENT_LENGTH"] = "";
    env["CONTENT_TYPE"] = m_script.mime;
    env["GATEWAY_INTERFACE"] = "CGI/1.1"; //@TODO: é esta a versão?
    env["PATH_INFO"] = m_script.cgi_info;
    env["PATH_TRANSLATED"] = "";
    env["QUERY_STRING"] = m_request.target_query();
    //@TODO: o construtor tem de receber a informação da ligação para fazer isto
    //@QUESTION: podemos receber esta informacao ou nao? de onde vem?
    env["REMOTE_ADDR"] = "";
    env["REMOTE_HOST"] = "";
    env["REMOTE_IDENT"] = "";
    env["REMOTE_USER"] = "";
    env["REQUEST_METHOD"] = m_request.method();
    env["SCRIPT_NAME"] = m_script.cgi_name;
    env["SERVER_NAME"] = m_ctx.config().service().server_name;
    env["SERVER_PORT"] = ""; //@TODO: porta do cliente
    env["SERVER_PROTOCOL"] = constants::server_http_version;
    env["SERVER_SOFTWARE"] = constants::server_name;

    // make http headers CGI compliant
    for (std::map<std::string, std::string>::const_iterator it = m_request.headers().begin();
         it != m_request.headers().end();
         ++it)
    {
        std::string key = to_uppercase_and_underscore(it->first);
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
        http_utils::throw_gateway_timeout(m_script.raw, m_timeout, m_ctx);
}
