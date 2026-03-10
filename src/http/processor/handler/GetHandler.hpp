#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "core/Path.hpp"
#include "http/response/Response.hpp"
#include "http/processor/handler/IRequestHandler.hpp"
#include "http/processor/handler/CgiHandler.hpp"

class GetHandler : public IRequestHandler 
{
	public:
		GetHandler(const Request& request, const RequestContext& ctx);
		void process();
		bool done() const;
		const Response& response() const;
		~GetHandler();

	private:
		const RequestContext& m_ctx;
		Response m_response;
		bool m_done;
		CgiHandler m_cgi;

		// utils
		void handle_index(Response& response, const Path& path);
		void handle_autoindex(Response& response, const Path& path);
		std::string make_autoindex(const Path& path);
		void handle_file(Response& response, const Path& path);
};

#endif // GETHANDLER_HPP
