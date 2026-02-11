#include <map>
#include <string>
#include <iostream>
#include <algorithm>

#include "core/constants.hpp"
#include "core/utils.hpp"
#include "core/MimeTypes.hpp"
#include "config/ConfigTypes.hpp"
#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const HttpRequest& request, const ServiceConfig& service, const Path& script)
	: m_request(request)
	, m_service(service)
	, m_env(init_env())
{
}

std::map<std::string, std::string> CgiHandler::init_env()
{
	std::map<std::string, std::string> env;

	// always have
	env["AUTH_TYPE"] = "";
	env["CONTENT_LENGTH"] = m_request.headers()["content-length"];
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
	env["SERVER_PORT"] = "";//m_service.listeners;
	env["SERVER_PROTOCOL"] = constants::server_http_version;
	env["SERVER_SOFTWARE"] = "";

	// make http headers cgi compliant
	for (std::map<std::string, std::string>::const_iterator it = m_request.headers().begin(); it != m_request.headers().end(); ++it)
	{
		//@TODO: criar uma func para toupper e replace em simultâneo
		std::string key = utils::str_toupper(it->first);
		std::replace(key.begin(), key.end(), '-', '_');
		env[key] = it->second;
		std::cout << key << ": " << it->second  << "\n";
	}
	return env;
}
