#include <map>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include "core/constants.hpp"
#include "core/utils.hpp"
#include "core/MimeTypes.hpp"
#include "core/Path.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpRequestContext.hpp"
#include "http/StatusCode.hpp"
#include "config/ConfigTypes.hpp"
#include "CgiHandler.hpp"
#include "NewHttpResponse.hpp"

CgiHandler::CgiHandler(const HttpRequest& request, const HttpRequestContext& ctx)
	: m_request(request)
	, m_ctx(ctx)
	, m_script(ctx.config().path())
	, m_env(init_env())
	, m_state(ForkExec)
{
}

void CgiHandler::process()
{
	switch (m_state)
	{
		case ForkExec:
		{
			fork_and_exec();
			m_state = ReadHeaders;
			break;
		}
		case ReadHeaders:
		{
			// headers are read from pipe to std::map
			m_state = read_pipe_chunk_headers();
			break;
		}
		case ReadBody:
		{
			parse_headers();
			// body is read directly to socket
			m_response.set_body_as_fd(m_fd[0], m_body_leftover);
			m_state = Done;
			break;
		}
		default:
		;
	}
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
		char* interpreter_raw_path = const_cast<char*>(interpreter.resolved.c_str());
		char* script_raw_path = const_cast<char*>(m_ctx.config().path().resolved.c_str());
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
		perror("execve() failed");
		exit(1);
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
	char buffer[constants::read_chunk_size];
	ssize_t bytes = read(m_fd[0], buffer, constants::read_chunk_size);
	if (bytes > 0)
	{
		buffer[bytes] = '\0';
		m_headers += buffer;
		if (m_headers.find("\r\n\r\n") != std::string::npos)
			return ReadBody;
	}
	else if (bytes == 0)
	{
		return ReadBody;
	}
	else
	{
		//@TODO error throw
		return Error;
	}
	return ReadHeaders;
}

bool CgiHandler::done() const
{
	return m_state == Done;
}

const NewHttpResponse& CgiHandler::response() const
{
	return m_response;
}

void CgiHandler::parse_headers()
{
	size_t status_code = 0;

	// if the final chunk over reads and consumes body characters
	// store them as leftovers
	size_t body_start = m_headers.find("\r\n\r\n");
	if (body_start != std::string::npos)
	{
		m_body_leftover = m_headers.substr(body_start + 4);
		m_headers = m_headers.substr(0, body_start);
	}

	// set headers
	std::vector<std::string> lines = utils::str_split(m_headers, "\r\n");
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
				status_code = std::atoi(value.c_str());

			m_response.set_header(key, value);
		}
		else
		{
			//@TODO: throw here 
		}
	}

	// if cgi defines the status code, set it in the response
	if (status_code)
	{
		//@TODO: pode dar um código que não seja válido?
		//StatusCode::is_valid()?
		m_response.set_status(static_cast<StatusCode::Code>(status_code));
	}
	else
	{
		m_response.set_status(StatusCode::Ok);
	}
}

std::map<std::string, std::string> CgiHandler::init_env()
{
	std::map<std::string, std::string> env;

	// default headers
	env["AUTH_TYPE"] = "";
	env["CONTENT_LENGTH"] = "";
	env["CONTENT_TYPE"] = m_script.mime;
	env["CONTENT_TYPE"] = "";
	env["GATEWAY_INTERFACE"] = "CGI/1.1"; //@TODO: é esta a versão?
	env["PATH_INFO"] = m_script.cgi_info;
	env["PATH_TRANSLATED"] = "";
	env["QUERY_STRING"] = m_request.target_query();
	//@TODO: o construtor tem de receber a informação da ligação para fazer isto
	env["REMOTE_ADDR"] = "";
	env["REMOTE_HOST"] = "";
	env["REMOTE_IDENT"] = "";
	env["REMOTE_USER"] = "";
	env["REQUEST_METHOD"] = m_request.method();
	env["SCRIPT_NAME"] = m_script.cgi_name;
	env["SERVER_NAME"] = m_ctx.config().service().server_name;
	env["SERVER_PORT"] = "";
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

CgiHandler::~CgiHandler() {}
