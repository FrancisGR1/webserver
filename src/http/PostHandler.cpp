#include <fstream>

#include "core/utils.hpp"
#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"
#include "HttpRequest.hpp"
#include "HttpResponseException.hpp"
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

	if (!config.allows_method("POST"))
	{
		throw ResponseError(
				StatusCode::MethodNotAllowed,
				"POST not allowed"
				);
	}

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
		// get upload dir path and check if it's uploadable
		Path upload_dir = config.upload_dir();
		is_uploadable(m_request, config, upload_dir);

		// create a name for the new file to be uploaded
		std::string file_name = utils::to_string(m_uploaded_file_index++) + ".data";
		Path upload = utils::join_paths(upload_dir.resolved, file_name);

		// open file
		std::ofstream file;
		file.open(upload.resolved.c_str());
		if (!file.is_open())
		{
			throw ResponseError(
					StatusCode::InternalServerError,
					utils::fmt("Failed when creating uploaded file: '%s'", upload.resolved.c_str())
					);
		}
		
		// write to file
		file << m_request.body();

		// make http m_response
		m_response.set_status(StatusCode::Created);
		// body
		std::string json = \
				   "{"
				   "\"status\": \"success\","
				   "\"filename\": \"" + upload.resolved + "\","
				   "\"size\": " + utils::to_string(upload.resolved.size()) +
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
		throw ResponseError(
				StatusCode::BadRequest,
				utils::fmt("Can't upload files at location: '%s'", config.path().resolved.c_str())
				);
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
void PostHandler::is_uploadable(const HttpRequest& request, const HttpRequestConfig& config, const Path& upload_dir) const
{
	if (!config.has_upload_dir())
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("Can't upload file: location '%s' is missing upload directory path", config.location()->name.c_str())
				);
	if (request.body().size() > config.max_body_size())
	{
		throw ResponseError(
				StatusCode::ContentTooLarge,
				utils::fmt("Request body size is larger than expected")
				);
	}
	if (!upload_dir.exists)
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("'%s' doesn't exist", upload_dir.resolved.c_str())
				);
	if (!upload_dir.is_directory)
		throw ResponseError(
				StatusCode::InternalServerError,
				utils::fmt("'%s' is not a directory", upload_dir.resolved.c_str())
				);
	if (!upload_dir.can_write || !upload_dir.can_execute)
		throw ResponseError(
				StatusCode::Forbidden,
				utils::fmt("'%s' upload directory doesn't have write and/or execution permission(s)", upload_dir.resolved.c_str())
				);
}

PostHandler::~PostHandler() {};
