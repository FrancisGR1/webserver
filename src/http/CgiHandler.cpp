#include <map>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "core/constants.hpp"
#include "core/utils.hpp"
#include "core/MimeTypes.hpp"
#include "core/Path.hpp"
#include "http/HttpRequest.hpp"
#include "config/ConfigTypes.hpp"
#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const HttpRequest& request, const ServiceConfig& service, const Path& script)
	: m_request(request)
	, m_service(service)
	, m_script(script)
	, m_env(init_env())
{
	exec();
}

const std::string& CgiHandler::get() const
{
	return m_output;
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
	env["SERVER_NAME"] = m_service.server_name;
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

void CgiHandler::exec()
{
	int fd[2];
	pipe(fd);
	pid_t id = fork();
	if (id == 0)
	{
		char* argv[] = { const_cast<char *>(m_script.cgi_path.c_str()), NULL };
		char* envp[m_env.size() + 1];
		size_t i = 0;
		for (std::map<std::string, std::string>::const_iterator it = m_env.begin(); it != m_env.end(); ++it)
		{
			std::string variable = it->first + "=" + it->second;
			envp[i] = strdup(variable.c_str());
			++i;
		}
		envp[i] = NULL;
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		close(fd[0]);
		// execve variables
		execve(m_script.cgi_path.c_str(), argv, envp);
		perror("execve() failed");
		exit(1);
	}
	else
	{
		close(fd[1]);
		char buffer[1000];
		ssize_t bytes;
		while ((bytes = read(fd[0], buffer, 1000)) > 0)
		{
			std::cout << "bytes read: " << bytes << "\n";
			buffer[bytes] = '\0';
			m_output += buffer;
		}
		close(fd[0]);
		waitpid(id, 0, 0);
		std::cout << "In pipe: '" << m_output << "'\n";
	}
}
