#include <map>
#include <string>
#include <iostream>

#include "core/constants.hpp"
#include "core/utils.hpp"
#include "core/MimeTypes.hpp"
#include "http/HttpRequest.hpp"
#include "config/ConfigTypes.hpp"
#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const HttpRequest& request, const ServiceConfig& service, const Path& script)
	: m_request(request)
	, m_service(service)
	, m_script(script)
	, m_env(init_env())
{
	for (std::map<std::string, std::string>::const_iterator it = m_env.begin(); it != m_env.end(); ++it)
		std::cout << it->first << ": " << it->second << "\n";
}

std::map<std::string, std::string> CgiHandler::init_env()
{
	std::map<std::string, std::string> env;

	// always have
	env["AUTH_TYPE"] = "";
	env["CONTENT_LENGTH"] = "";
	env["CONTENT_TYPE"] = m_script.mime;
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
	
	// make http headers cgi compliant
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
