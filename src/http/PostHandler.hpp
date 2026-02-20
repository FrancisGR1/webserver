#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "core/Path.hpp"
#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestConfig.hpp"
#include "NewHttpResponse.hpp"
#include "AMethodHandler.hpp"
#include "CgiHandler.hpp"

class PostHandler : public AMethodHandler 
{
	public:
		PostHandler(const HttpRequest& request, const HttpRequestContext& ctx);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;
		~PostHandler();
	private:
		const HttpRequest& m_request; 
		const HttpRequestContext& m_ctx;
		NewHttpResponse m_response;
		bool m_done;
		const CgiHandler m_cgi;
		static unsigned long long m_uploaded_file_index;

		void is_uploadable(const HttpRequest& request, const HttpRequestConfig& config, const Path& upload_dir) const;
};

#endif // POSTHANDLER_HPP
