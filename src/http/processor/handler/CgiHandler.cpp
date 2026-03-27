#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include "core/constants.hpp"
#include "core/utils.hpp"
#include "core/Path.hpp"
#include "http/http_utils.hpp"
#include "http/request/Request.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/StatusCode.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/response/Response.hpp"

CgiHandler::CgiHandler(const Request& request, const RequestContext& ctx)
	: m_request(request)
	, m_ctx(ctx)
	, m_script(ctx.config().path())
	, m_response(StatusCode::Ok) //@ASSUMPTION: default to 200
	, m_env(init_env())
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
			m_timer.set(constants::cgi_timeout);
			m_timer.start();
			m_state = ForkExec;
		}
		// fall through

		case ForkExec:
		{
			Logger::trace("CgiHandler: fork and execute");
			fork_and_exec();
			m_state = ReadHeaders;
			break;
		}

		case ReadHeaders:
		{
			Logger::trace("CgiHandler: read headers");
			// headers are read from pipe to std::map
			m_state = read_pipe_chunk_headers();
			break;
		}

		case ReadBody:
		{
			parse_headers();
			Logger::trace("CgiHandler: headers:'%s'", m_headers.c_str());

			Logger::trace("CgiHandler: read body");
			// body is read directly to socket
			m_response.set_body_as_fd_and_prefix(m_fd[0], m_body_leftover);

			// finish
			m_timer.stop();
			m_state = Done;
			break;
		}
		default:
		;
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

void CgiHandler::fork_and_exec()
{	
	pipe(m_fd);
	pid_t id = fork();
	if (id == 0)
	{
		//@TODO @QUESTION o const cast é necessário?
		// build argv
		Path interpreter = m_ctx.config().cgi_interpreter();
		char* interpreter_raw_path = const_cast<char*>(interpreter.raw.c_str());
		char* script_raw_path = const_cast<char*>(m_ctx.config().path().raw.c_str());
		Logger::trace("CgiHandler: Child argv: '%s' '%s'", interpreter_raw_path, script_raw_path);
		char* argv[] = { interpreter_raw_path, script_raw_path, NULL };

		// build env
		std::vector<std::string> env_strings;
		std::vector<char*> envp;

		for (std::map<std::string,std::string>::const_iterator it = m_env.begin();
				it != m_env.end(); ++it)
		{
			env_strings.push_back(it->first + "=" + it->second);
		}

		for (size_t i = 0; i < env_strings.size(); ++i)
			envp.push_back(const_cast<char*>(env_strings[i].c_str()));
		envp.push_back(NULL);

		dup2(m_fd[1], STDOUT_FILENO);
		dup2(m_fd[1], STDERR_FILENO);

		close(m_fd[1]);
		close(m_fd[0]);

		// execve variables
		execve(argv[0], argv, &envp[0]);
		perror("execve()");
	}
	else
	{
		//@TODO: add fd [0] to events pool
		close(m_fd[1]);
		fcntl(m_fd[0], F_SETFL, O_NONBLOCK);
		waitpid(id, 0, WNOHANG); // don't wait for subprocess
	}
}

CgiHandler::State CgiHandler::read_pipe_chunk_headers()
{
	char buffer[constants::read_chunk_size + 1];
	ssize_t bytes = read(m_fd[0], buffer, constants::read_chunk_size);
	Logger::trace("CgiHandler: read %ld from pipe", bytes);
	if (bytes > 0)
	{
		buffer[bytes] = '\0';
		m_headers += buffer;
		Logger::trace("CgiHandler: read buffer:'%s'", buffer); 
		if (m_headers.find(constants::crlfcrlf) != std::string::npos)
			return ReadBody;
	}
	else if (bytes == 0)
	{
		return ReadBody;
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) //@TMP @WARN não podemos usar errno
		{
			Logger::trace("CgiHandler: pipe empty");
			return ReadHeaders;  // pipe empty but child still running — try again later
		}

		Logger::warn("CgiHandler: read() call from pipe failed!");
		//@TODO: definir erro status
		return Error;
	}
	return ReadHeaders;
}
// https://www.rfc-editor.org/rfc/rfc3875#section-6
void CgiHandler::parse_headers()
{
	Logger::trace("CgiHandler: transform headers to a string");

	// if the final chunk over reads and consumes body characters
	// store them as leftovers
	size_t body_start = m_headers.find(constants::crlfcrlf);
	if (body_start != std::string::npos)
	{
		m_body_leftover = m_headers.substr(body_start + 4);
		m_headers = m_headers.substr(0, body_start);
	}

	// set headers
	std::vector<std::string> lines = utils::str_split(m_headers, constants::crlf);
	for (size_t i = 0; i < lines.size(); ++i)
	{
		const std::string& line = lines[i];
		size_t colon_pos = line.find(":");
		if (colon_pos != std::string::npos)
		{
			std::string key = line.substr(0, colon_pos);
			utils::str_trim_sides(key, " \t");

			std::string value = line.substr(colon_pos + 1);
			utils::str_trim_sides(value, " \t");

			if (key == "Status")
				m_response.set_status(static_cast<StatusCode::Code>(std::atoi(value.c_str())));

			m_response.set_header(key, value);
		}
		else
		{
			//@TODO: throw here 
		}
	}
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
	for (std::map<std::string, std::string>::const_iterator it = m_request.headers().begin(); it != m_request.headers().end(); ++it)
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
		char c = str[i] == '-'
			? '_'
			: std::toupper(str[i]);

		result[i] = c;
	}
	return result;
}

void CgiHandler::expect_has_time_left() const
{
	if (m_timer.expired())
		http_utils::throw_gateway_timeout(m_script.raw, m_ctx);
}
