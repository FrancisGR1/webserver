#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <map>
#include <string>

#include "core/Path.hpp"
#include "config/types/ServiceConfig.hpp"
#include "http/request/Request.hpp"
#include "config/ConfigTypes.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/processor/handler/CgiHandler.hpp"
#include "http/processor/RequestContext.hpp"
#include "http/response/Response.hpp"

// https://datatracker.ietf.org/doc/html/rfc3875
class CgiHandler : public IRequestHandler
{
	public:
		CgiHandler(const Request& request, const RequestContext& ctx);
		void process();
		bool done() const;
		const Response& response() const;
		~CgiHandler();

	private:
		enum State 
		{
			Error = -1,
			StartTimer,
			ForkExec,
			ReadHeaders,
			ReadBody,
			Done
		};

		const Request& m_request; 
		const RequestContext& m_ctx; 
		const Path& m_script;
		Response m_response;
		Timer m_timer;
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
