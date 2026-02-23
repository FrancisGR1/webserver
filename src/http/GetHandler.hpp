#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "core/Path.hpp"
#include "config/ConfigTypes.hpp"
#include "NewHttpResponse.hpp"
#include "HttpRequestConfig.hpp"
#include "IRequestHandler.hpp"
#include "CgiHandler.hpp"

class GetHandler : public IRequestHandler 
{
	public:
		GetHandler(const HttpRequest& request, const HttpRequestContext& ctx);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;
		~GetHandler();

	private:

		const HttpRequest& m_request; 
		const HttpRequestContext& m_ctx;
		NewHttpResponse m_response;
		bool m_done;
		CgiHandler m_cgi;

		// utils
		void handle_index(NewHttpResponse& response, const Path& path);
		void handle_autoindex(NewHttpResponse& response, const Path& path);
		std::string make_autoindex(const Path& path);
		void handle_file(NewHttpResponse& response, const Path& path);
};

#endif // GETHANDLER_HPP
