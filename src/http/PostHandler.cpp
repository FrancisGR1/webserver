#include <unistd.h>
#include <fcntl.h>

#include "core/utils.hpp"
#include "core/constants.hpp"
#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"
#include "HttpRequest.hpp"
#include "http_utils.hpp"
#include "HttpRequestConfig.hpp"
#include "HttpRequestContext.hpp"
#include "PostHandler.hpp"

unsigned long long PostHandler::m_uploaded_file_index;

PostHandler::PostHandler(const HttpRequest& request, const HttpRequestContext& ctx) 
	: m_request(request)
	, m_ctx(ctx)
	, m_done(false)
	, m_cgi(request, ctx)
	, m_upload(NULL)
	, m_fd(-1) {}

static void is_uploadable_precondition(const HttpRequest& request, const HttpRequestConfig& config, const Path& upload_dir)
{
	if (!config.has_upload_dir())
		http_utils::throw_internal_server_error_cant_upload();
	if (request.body().size() > config.max_body_size()) 
		http_utils::throw_content_too_large();
	if (!upload_dir.exists) 
		http_utils::throw_internal_server_error_doesnt_exist(upload_dir);
	if (!upload_dir.is_directory) 
		http_utils::throw_internal_server_error_not_a_directory(upload_dir);
	if (!upload_dir.can_write || !upload_dir.can_execute) 
		http_utils::throw_forbidden_cant_upload(upload_dir);
}

void PostHandler::process()
{
	const HttpRequestConfig& config = m_ctx.config();

	if (!config.allows_method("POST")) http_utils::throw_method_not_allowed("POST");

	if (config.is_redirected())
	{
		m_response.set_status(StatusCode::MovedPermanently);
		m_response.set_header("Location", config.redirection().path);
		m_response.set_header("Connection", "close");
		m_response.set_header("Date", utils::http_date());
		m_done = true;
	}
	else if (config.is_cgi())
	{
		m_cgi.process();
		if (m_cgi.done())
			m_done = true;
		return;
	}
	else if (config.allows_upload())
	{
		if (m_fd == -1) // set upload dir
		{
			Path upload_dir = config.upload_dir();
			// check if it's uploadable
			is_uploadable_precondition(m_request, config, upload_dir);
			// create a name for the new file to be uploaded
			std::string file_name = utils::to_string(m_uploaded_file_index++) + ".data";
			// make upload real path
			m_upload = utils::join_paths(upload_dir.resolved, file_name);
			//@TODO add fd to event pool
			m_fd = open(m_upload.resolved.c_str(), O_WRONLY | O_APPEND);
		}

		if (!m_done)
		{
			size_t to_write = (m_request.body().size() - m_offset) > constants::write_chunk_size
				? constants::write_chunk_size
				: m_request.body().size() - m_offset;
			ssize_t written = ::write(m_fd, m_request.body().c_str() + m_offset, to_write);
			if (written >= 0)
				m_offset += written;
			//@TODO apanhar erro em caso de -1
			if (m_offset == static_cast<ssize_t>(m_request.body().size()))
				m_done = true;
		}

		if (m_done)
		{
			// status
			m_response.set_status(StatusCode::Created);

			// body
			std::string json = \
					   "{"
					   "\"status\": \"success\","
					   "\"filename\": \"" + m_upload.resolved + "\","
					   "\"size\": " + utils::to_string(m_offset) +
					   "}";
			m_response.set_body_as_str(json);

			// headers
			m_response.set_header("Location", m_upload.resolved);
			m_response.set_header("Connection", "close"); // @NOTE: HTTP1.0 closes by default;
			m_response.set_header("Date", utils::http_date());
			m_response.set_header("Content-Type", "application/json");
			m_response.set_header("Content-Length", utils::to_string(json.size()));

		}
	}
	else
	{
		//@TODO: que código de erro é aqui? 404?
		http_utils::throw_internal_server_error_cant_upload();
	}
}

bool PostHandler::done() const
{
	return m_done;
}

const NewHttpResponse& PostHandler::response() const
{
	return m_response;
}

PostHandler::~PostHandler() { if (m_fd > -1) close(m_fd); };
