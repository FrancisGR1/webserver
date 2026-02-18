#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "core/Path.hpp"
#include "config/ConfigTypes.hpp"
#include "HttpRequestConfig.hpp"
#include "HttpResponse.hpp"
#include "AMethodHandler.hpp"

class PostHandler : public AMethodHandler 
{
	public:
		PostHandler();
		ssize_t send(int socket_fd) const;
		HttpResponse handle(const HttpRequest& request, const HttpRequestContext& ctx) const;
		~PostHandler();
	private:
		static unsigned long long m_uploaded_file_index;

		void is_uploadable(const HttpRequest& request, const HttpRequestConfig& config, const Path& upload_dir) const;
};

#endif // POSTHANDLER_HPP
