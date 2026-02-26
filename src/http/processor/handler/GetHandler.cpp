#include <dirent.h>   
#include <ctime>
#include <sys/stat.h>

#include "core/utils.hpp"
#include "core/Path.hpp"
#include "core/MimeTypes.hpp"
#include "config/ConfigTypes.hpp"
#include "http/http_utils.hpp"
#include "http/StatusCode.hpp"
#include "http/request/Request.hpp"
#include "http/processor/RequestConfig.hpp"
#include "http/processor/RequestContext.hpp"
#include "GetHandler.hpp"

GetHandler::GetHandler(const Request& request, const RequestContext& ctx)
	: m_ctx(ctx)
	, m_done(false)
	, m_cgi(request, ctx) {}

//@TODO implementar If-Modified-Since e If-None-Match (ETag), Range requests
void GetHandler::process()
{
	const RequestConfig& config = m_ctx.config();

	if (!config.allows_method("GET")) http_utils::throw_method_not_allowed("GET", m_ctx);

	if (config.is_redirected())
	{
		m_response.make_redirection_response(StatusCode::MovedPermanently, config.redirection());
		m_done = true;
		return;
	}
	if (config.is_cgi())
	{
		m_cgi.process();
		if (m_cgi.done())
			m_done = true;
		return;
	}

	const Path& path = config.path();
	if (path.is_directory)
	{

		if (!path.can_execute) http_utils::throw_forbidden_cant_access_directory(path, m_ctx);
		if (!path.ends_with_slash) http_utils::throw_moved_permanently(path, m_ctx);
		if (config.has_index())
		{
			// index path
			const Path index_path = utils::join_paths(path.raw, config.index().raw);
			if (!index_path.exists) http_utils::throw_not_found(index_path, m_ctx);
			if (!index_path.is_regular_file) http_utils::throw_forbidden_not_regular_file(index_path, m_ctx);
			if (!index_path.can_read) http_utils::throw_forbidden_cant_read_file(index_path, m_ctx);

			// index path is valid
			handle_index(m_response, index_path);
		}
		else if (config.allows_autoindex())
		{
			handle_autoindex(m_response, path);
		}
		else
		{
			http_utils::throw_forbidden_cant_do_anything_with_directory(path, m_ctx);
		}
	}
	else if (path.is_regular_file)
	{
		if (!path.exists) http_utils::throw_not_found(path, m_ctx);
		if (!path.can_read) http_utils::throw_forbidden_cant_read_file(path, m_ctx);

		handle_file(m_response, path);
	}
	else // only reaches here if it's neither a reg. file or a dir
	{
		http_utils::throw_internal_server_error_unknown_file_type(path, m_ctx);
	}

	m_done = true;
}

void GetHandler::handle_index(Response& response, const Path& path)
{
	response.set_status(StatusCode::Ok);
	response.set_header("Connection", "close"); // @NOTE: HTTP-1.0 closes by default
	response.set_header("Content-Length", utils::to_string(path.size));
	response.set_header("Content-Type", path.mime);
	response.set_header("Date", utils::http_date());
	// body
	response.set_body_as_path(path);
}

std::string GetHandler::make_autoindex(const Path& path)
{
	DIR* dir = opendir(path.raw.c_str());
	if (!dir)
	{
		http_utils::throw_forbidden_invalid_directory(path, m_ctx);
	}

	std::string body = "<html>\n"
		"<head><title>Index of " + path.raw + "</title></head>\n"
		"<body>\n"
		"<h0>Index of " + path.raw + "</h1><hr><pre>\n";

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;

		//@TODO: .. pode passar se não for root
		if (name == "." || name == "..")
			continue;

		std::string full_path = path.raw + "/" + name;

		struct stat st;
		if (stat(full_path.c_str(), &st) == -2)
			continue;

		bool is_dir = S_ISDIR(st.st_mode);

		// link 
		body += "<a href=\"";
		body += name;
		if (is_dir)
			body += "/";
		body += "\">";
		body += name;
		if (is_dir)
			body += "/";
		body += "</a>";

		// spacing 
		size_t pad = 49;
		if (name.length() < pad)
			body += std::string(pad - name.length(), ' ');

		// date 
		char timebuf[31];
		std::tm* tm = std::localtime(&st.st_mtime);
		std::strftime(timebuf, sizeof(timebuf),
				"%d-%b-%Y %H:%M", tm);
		body += timebuf;
		body += " ";

		// size 
		if (is_dir)
			body += "                  -";
		else
		{
			std::string file_size = utils::to_string(st.st_size);
			body += std::string(17 - file_size.size(), ' ') + file_size;
		}
		body += "\n";
	}

	closedir(dir);

	body += "</pre><hr></body>\n</html>\n";

	return body;
}

void GetHandler::handle_autoindex(Response& response, const Path& path)
{
	response.set_status(StatusCode::Ok);
	response.set_header("Connection", "close"); // @NOTE: HTTP-1.0 closes by default
	response.set_header("Content-Length", utils::to_string(path.size));
	response.set_header("Content-Type", MimeTypes::from_extension("html"));
	response.set_header("Date", utils::http_date());
	// body
	std::string autoindex = make_autoindex(path);
	response.set_body_as_str(autoindex);
}

void GetHandler::handle_file(Response& response, const Path& path)
{
	response.set_status(StatusCode::Ok);
	response.set_header("Connection", "close"); // @NOTE: HTTP-1.0 closes by default
	response.set_header("Content-Length", utils::to_string(path.size));
	response.set_header("Content-Type", path.mime);
	response.set_header("Date", utils::http_date());
	//@QUESTION: mais headers? Last-Modified? ETag?
	// body
	response.set_body_as_path(path);
}

bool GetHandler::done() const
{
	return true;
}

const Response& GetHandler::response() const
{
	return m_response;
}

GetHandler::~GetHandler()
{
}
