#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "core/Path.hpp"
#include "config/ConfigTypes.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestContext.hpp"
#include "HttpRequestConfig.hpp"
#include "NewHttpResponse.hpp"
#include "IRequestHandler.hpp"
#include "CgiHandler.hpp"

class PostHandler : public IRequestHandler 
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
		CgiHandler m_cgi;

		// upload
		Path m_upload;
		int m_fd;
		ssize_t m_offset;
		static unsigned long long m_uploaded_file_index;

		// utils
		ssize_t handle_upload(const std::string& body, size_t offset, int fd);
		void is_uploadable_precondition(const HttpRequest& request, const HttpRequestConfig& config, const Path& upload_dir);
};

#endif // POSTHANDLER_HPP
