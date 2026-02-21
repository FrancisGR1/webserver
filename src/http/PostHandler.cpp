#include <fstream>

#include "core/utils.hpp"
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
{}

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
		//@TODO: setup cgi here?
	}
	else if (config.allows_upload())
	{
		// get upload dir path
		Path upload_dir = config.upload_dir();

		// check if it's uploadable
		is_uploadable_precondition(m_request, config, upload_dir);

		// create a name for the new file to be uploaded
		std::string file_name = utils::to_string(m_uploaded_file_index++) + ".data";
		Path upload = utils::join_paths(upload_dir.resolved, file_name);

		// open file
		std::ofstream file;
		file.open(upload.resolved.c_str()) http_utils::throw_internal_server_error_failed_upload(upload);

		// write to file
		// @TODO: fazer com que seja async compatible
		file << m_request.body();



		// --------------------
		// --------------------
		// --------------------
		// make http m_response
		// --------------------
		// --------------------
		// --------------------
		m_response.set_status(StatusCode::Created);

		// body
		std::string json = \
				   "{"
				   "\"status\": \"success\","
				   "\"filename\": \"" + upload.resolved + "\","
				   "\"size\": " + utils::to_string(m_request.body().size()) +
				   "}";
		m_response.set_body_as_str(json);

		// headers
		m_response.set_header("Location", upload.resolved);
		m_response.set_header("Connection", "close"); // @NOTE: HTTP1.0 closes by default;
		m_response.set_header("Date", utils::http_date());
		m_response.set_header("Content-Type", "application/json");
		m_response.set_header("Content-Length", utils::to_string(json.size()));

		m_done = true;
	}
	else
	{
		//@TODO: que código de erro é aqui?
		throw_internal_server_error_cant_upload(config.path());
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

// @TODO isto não pode pertencer a Path? Path file; file.is_uploadable() é mais limpo;
void PostHandler::is_uploadable_precondition(const HttpRequest& request, const HttpRequestConfig& config, const Path& upload_dir) const
{
	if (!config.has_upload_dir())
		http_utils::throw_internal_server_error_cant_upload(upload_dir);
	if (request.body().size() > config.max_body_size()) 
		http_utils::throw_content_too_large();
	if (!upload_dir.exists) 
		http_utils::throw_internal_server_error_doesnt_exist(upload_dir);
	if (!upload_dir.is_directory) 
		http_utils::throw_internal_server_error_not_a_directory(upload_dir);
	if (!upload_dir.can_write || !upload_dir.can_execute) 
		http_utils::throw_forbidden_cant_upload(upload_dir);
}

PostHandler::~PostHandler() {};
