#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <map>
#include <string>

#include "core/Path.hpp"
#include "config/types/ServiceConfig.hpp"
#include "http/HttpRequest.hpp"
#include "config/ConfigTypes.hpp"
#include "IRequestHandler.hpp"
#include "CgiHandler.hpp"
#include "HttpRequestContext.hpp"
#include "NewHttpResponse.hpp"

// https://datatracker.ietf.org/doc/html/rfc3875
class CgiHandler : public IRequestHandler
{
	public:
		CgiHandler(const HttpRequest& request, const HttpRequestContext& ctx);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;
		~CgiHandler();

	private:
		enum State 
		{
			Error = -1,
			ForkExec,
			ReadHeaders,
			ReadBody,
			Done
		};

		const HttpRequest& m_request; 
		const HttpRequestContext& m_ctx; 
		const Path& m_script;
		NewHttpResponse m_response;

		// cgi
		std::map<std::string, std::string> m_env;
		int m_fd[2];
		std::string m_headers;
		std::string m_body_leftover;
		State m_state;

		// util
		std::map<std::string, std::string> init_env();
		std::string to_uppercase_and_underscore(const std::string& str);
		void parse_headers();
		State read_pipe_chunk_headers();
		void fork_and_exec();


};

#endif //CGI_HANDLER_HPP
