#include <fstream>

#include "core/utils.hpp"
#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestConfig.hpp"
#include "HttpRequestContext.hpp"
#include "PostHandler.hpp"

unsigned long long PostHandler::m_uploaded_file_index;

PostHandler::PostHandler() {};

ssize_t PostHandler::send(int socket_fd) const
{
	(void) socket_fd;
	return 1;
}

HttpResponse PostHandler::handle(const HttpRequest& request, const HttpRequestContext& ctx) const
{
	const HttpRequestConfig& config = ctx.config();

	if (!config.allows_method("POST"))
	{
		throw HttpResponseException(
				StatusCode::MethodNotAllowed,
				"POST not allowed"
				);
	}

	if (config.is_redirected())
	{
		// @TODO redirecionar
	}
	else if (config.is_cgi())
	{
	}
	else if (config.allows_upload())
	{
		// get upload dir path and check if it's uploadable
		Path upload_dir = config.upload_dir();
		is_uploadable(request, config, upload_dir);

		// create a name for the new uploaded file
		std::string file_name = utils::to_string(m_uploaded_file_index++) + ".data";
		std::string upload_path = utils::join_paths(upload_dir.resolved, file_name);

		// open file
		std::ofstream file;
		file.open(upload_path.c_str());
		if (!file.is_open())
		{
			throw HttpResponseException(
					StatusCode::InternalServerError,
					utils::fmt("Failed when creating uploaded file: '%s'", upload_path.c_str())
					);
		}
		
		// write to file
		file << request.body();


		//@TODO: AQUI - imlpementar http response
	}
	else
	{
		//@TODO: que código de erro é aqui?
		throw HttpResponseException(
				StatusCode::BadRequest,
				utils::fmt("Can't upload files at location: '%s'", config.path().resolved.c_str()),
				*config.location()
				);
	}

	// should never reach here
	return HttpResponse(request, ctx.config().service());
}

// @TODO isto não pode pertencer a Path? Path file; file.is_uploadbale() é mais limpo;
void PostHandler::is_uploadable(const HttpRequest& request, const HttpRequestConfig& config, const Path& upload_dir) const
{
	if (!config.has_upload_dir())
		throw HttpResponseException(
				StatusCode::InternalServerError,
				utils::fmt("Can't upload file: location '%s' is missing upload directory path", config.location()->name.c_str()),
				*config.location()

				);
	if (request.body().size() > config.max_body_size())
	{
		throw HttpResponseException(
				StatusCode::ContentTooLarge,
				utils::fmt("Request body size is larger than expected"),
				*config.location()
				);
	}
	if (!upload_dir.exists)
		throw HttpResponseException(
				StatusCode::InternalServerError,
				utils::fmt("'%s' doesn't exist", upload_dir.resolved.c_str())
				);
	if (!upload_dir.is_directory)
		throw HttpResponseException(
				StatusCode::InternalServerError,
				utils::fmt("'%s' is not a directory", upload_dir.resolved.c_str())
				);
	if (!upload_dir.can_write || !upload_dir.can_execute)
		throw HttpResponseException(
				StatusCode::Forbidden,
				utils::fmt("'%s' upload directory doesn't have write and/or execution permission(s)", upload_dir.resolved.c_str())
				);
}

PostHandler::~PostHandler() {};
