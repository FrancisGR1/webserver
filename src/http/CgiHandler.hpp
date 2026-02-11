#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <map>
#include <string>

#include "core/Path.hpp"
#include "http/HttpRequest.hpp"
#include "config/ConfigTypes.hpp"
#include "CgiHandler.hpp"

// https://datatracker.ietf.org/doc/html/rfc3875
class CgiHandler
{
	public:
	private:
		const HttpRequest& m_request; 
		const ServiceConfig& m_service; 
		const Path& m_script;
		std::map<std::string, std::string> m_env;

		std::map<std::string, std::string> init_default_env();
};

/*
 * Teoria
 * O que é o CGI?
 * Inteface para correr programas. Os programas a ser corridos vão esperar um conjunto de dados pré-definidos
 * O CGI faz com que o script e o servidor partilhem responsabilidades para responder aos pedidos
 * O CGI define as meta-variáveis que descrevem o pedido do cliente de tal modo a funcionar independentemente da plataforma
 *
 * Planeamento da implementação
 * * Verificar se tem permissões de execução (700)
 * * Definir ENV -> converter o pedido num pedido de cgi
 * * * Variáveis vazias = variáveis nulas
 * * * Variáveis são sensíveis a minúsculas/maiúsculas
 * * execução do script
 * * conversão da resposta cgi numa resposta http
 */

#endif //CGI_HANDLER_HPP
