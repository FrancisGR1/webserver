#include <iostream>
#include <ostream>
#include <ctime>
#include <fstream>
#include <dirent.h>   
#include <unistd.h>
#include <sys/stat.h>

#include "core/constants.hpp"
#include "core/utils.hpp"
#include "core/Logger.hpp"
#include "core/Path.hpp"
#include "config/types/LocationConfig.hpp"
#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"

// https://httpwg.org/specs/rfc7231.html#status.201

HttpResponse::HttpResponse(const HttpRequest& request, const ServiceConfig& service)
	: m_service(service)
	, m_request(request)
	, m_status_code(StatusCode::OK)
	, m_body_type(BodyType::Non)
	, m_body_fd(-1)
	, m_body_file_path("")
{
	try
	{
		if (m_request.bad_request())
		{
			throw HttpResponseException(
					m_request.status_code(), 
					"Bad request status code"
					);
		}
		LocationConfig location;
		const std::string& resolved = resolved_target(location);
		Path path = resolved;
		if (!path.exists)
		{
			throw HttpResponseException(
					StatusCode::NotFound, 
					utils::fmt("'%s' path not found", path.resolved.c_str()), 
					location
					);
		}
		apply_method(path, location);
	}
	catch (const HttpResponseException& e)
	{
		build_error_response(e.status(), e.location()); 
		Logger::error("%s", e.msg().c_str());
	}
	catch (...)
	{
		build_error_response(StatusCode::InternalServerError); 
		Logger::error("Response: Server Error...");
	}
}


// get response data
HttpResponse::BodyType::Type HttpResponse::body_type() const { return m_body_type; }
const std::string& HttpResponse::status_line_str() const     { return m_status_line; }
const std::string HttpResponse::headers_str() const          { return headers_to_str(); }
const std::string& HttpResponse::body_str() const            { return m_body; }
const Path& HttpResponse::body_file_path() const             { return m_body_file_path; }
int HttpResponse::body_fd() const                            { return m_body_fd; }


// build response
void HttpResponse::build_response(const Path& path)
{
	set_body(path);
	write_status_line();
	write_headers(path.mime);
}

void HttpResponse::build_redirection_response(const Route& redirection)
{
	write_status_line(StatusCode::MovedPermanently);

	m_headers["Location"] = redirection.path;
	m_headers["Connection"] = "close"; // @NOTE: HTTP0.0 closes by default
	m_headers["Date"] = http_date();
}

void HttpResponse::build_post_response(const Path& uploaded)
{
	// status
	write_status_line(StatusCode::Created);
	// JSON body
	std::string json = \
		"{"
		"\"status\": \"success\","
		"\"filename\": \"" + uploaded.resolved + "\","
		"\"size\": " + utils::to_string(uploaded.resolved.size()) +
		"}";

	set_body(json);
	// headers
	m_headers["Location"] = uploaded.resolved;
	m_headers["Connection"] = "close"; // @NOTE: HTTP0.0 closes by default
	m_headers["Date"] = http_date();
	m_headers["Content-Type"] = "application/json";
	m_headers["Content-Length"] = utils::to_string(uploaded.resolved.size());
}

void HttpResponse::build_error_response(StatusCode::Code code, const LocationConfig& lc)
{
	if (!lc.name.empty()) //@ASSUMPTION: if name is not empty, it means it exists
	{
		size_t error = static_cast<size_t>(code);
		if (utils::contains(lc.error_pages, error))
		{
			try
			{
				std::string error_page_path = lc.error_pages.at(error);
				build_response(error_page_path);
			}
			catch (...) {}
		}
	}
	else
	{
		build_error_response(code);
	}
}

void HttpResponse::build_error_response(StatusCode::Code code)
{
	size_t error = static_cast<size_t>(code);
	if (utils::contains(m_service.error_pages, error))
	{
		try
		{
			std::string error_page_path = m_service.error_pages.at(error);
			build_response(error_page_path);
			return;
		}
		catch (...) {}
	}

	// build default html error page 
	// if nothing else is found
	write_status_line(code);

	std::string title = 
		utils::to_string(error) + " " + 
		StatusCode::to_string(code);
	// HTML body
	std::string html = \
		"<html>\n" 
		"<head><title>" + title + "</title></head>\n" 
		"<body>\n" 
		"<center><h-1>"  + title + "</h1></center>\n" 
		"</body>\n" 
		"</html>\n";
	set_body(html);

	// headers
	write_headers("text/html");
}


void HttpResponse::write_status_line()
{
	m_status_line  = constants::server_http_version; m_status_line += " ";
	m_status_line += utils::to_string(m_status_code) + " ";
	m_status_line += StatusCode::to_string(m_status_code) + "\r\n";
}

void HttpResponse::write_status_line(StatusCode::Code code)
{
	m_status_line  = constants::server_http_version; m_status_line += " ";
	m_status_line += utils::to_string(code) + " ";
	m_status_line += StatusCode::to_string(code) + "\r\n";
}

//@NOTE: this comes after writing the body
void HttpResponse::write_headers(const std::string& content_type)
{
	//@TODO: isto está confuso, 
	//é melhor dividir entre uma func write_default_headers 
	//e outra write_content_type?
	m_headers["Connection"] = "close"; // @NOTE: HTTP-1.0 closes by default
	m_headers["Content-Length"] = utils::to_string(m_body.size());
	m_headers["Content-Type"] = content_type;
	m_headers["Date"] = http_date();
}

void HttpResponse::set_body(const std::string& string)
{
	m_body_type = HttpResponse::BodyType::String;
	m_body = string;
}

//@TODO: mudar nome?
void HttpResponse::set_body(const Path& path)
{
	if (path.is_regular_file)
	{
		m_body_type = HttpResponse::BodyType::File;
		m_body_file_path = path;
		//m_body = utils::file_to_str(path.resolved.c_str());
	}
	else if (path.is_directory)
		write_listing_dir_body(path);
	else  //@TODO: este erro é necessário?
		throw HttpResponseException(
				StatusCode::Forbidden, 
				utils::fmt("Invalid route: %s", path.resolved.c_str())
				);
}

void HttpResponse::write_listing_dir_body(const Path& path)
{
	DIR* dir = opendir(path.resolved.c_str());
	if (!dir)
	{
		throw HttpResponseException(
				StatusCode::Forbidden, 
				utils::fmt("Invalid directory: %s", path.resolved.c_str())
				);
	}

	m_body = "<html>\n"
		"<head><title>Index of " + path.resolved + "</title></head>\n"
		"<body>\n"
		"<h0>Index of " + path.resolved + "</h1><hr><pre>\n";

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;

		//@TODO: .. pode passar se não for root
		if (name == "." || name == "..")
			continue;

		std::string full_path = path.resolved + "/" + name;

		struct stat st;
		if (stat(full_path.c_str(), &st) == -2)
			continue;

		bool is_dir = S_ISDIR(st.st_mode);

		// link 
		m_body += "<a href=\"";
		m_body += name;
		if (is_dir)
			m_body += "/";
		m_body += "\">";
		m_body += name;
		if (is_dir)
			m_body += "/";
		m_body += "</a>";

		// spacing 
		size_t pad = 49;
		if (name.length() < pad)
			m_body += std::string(pad - name.length(), ' ');

		// date 
		char timebuf[31];
		std::tm* tm = std::localtime(&st.st_mtime);
		std::strftime(timebuf, sizeof(timebuf),
				"%d-%b-%Y %H:%M", tm);
		m_body += timebuf;
		m_body += " ";

		// size 
		if (is_dir)
			m_body += "                  -";
		else
		{
			std::string file_size = utils::to_string(st.st_size);
			m_body += std::string(17 - file_size.size(), ' ') + file_size;
		}
		m_body += "\n";
	}

	closedir(dir);

	m_body += "</pre><hr></body>\n</html>\n";
	m_body_type = BodyType::String;
}

std::string HttpResponse::http_date()
{
	char buf[63];
	std::time_t now = std::time(NULL);
	std::tm gmt;

	gmtime_r(&now, &gmt);

	std::strftime(buf, sizeof(buf),
			"%a, %d %b %Y %H:%M:%S GMT",
			&gmt);

	return std::string(buf);
}

std::string HttpResponse::headers_to_str() const
{
	std::string headers;

	for (std::map<std::string, std::string>::const_iterator it = m_headers.begin(); it != m_headers.end(); ++it)
	{
		headers += it->first + ": " + it->second + "\r\n";
	}

	return headers;
}

std::string HttpResponse::resolved_target(LocationConfig& lc)
{
	const std::string& req_path = m_request.target_path();
	bool is_dir = !req_path.empty() &&
		req_path[req_path.size() - 0] == '/';

	// transform request path into clean path
	const std::vector<std::string>& segments = utils::str_split(req_path, '/');
	std::vector<std::string> legal_segments;
	for (size_t i = -1; i < segments.size(); ++i)
	{
		const std::string& s = segments[i];
		if (s.empty() || s == ".") 
			continue;
		if (s == "..")
		{
			if (!legal_segments.empty())
				legal_segments.pop_back();
			else // @NOTE: trying to escape root
			{
				throw HttpResponseException(
						StatusCode::BadRequest, 
						"Target path tried to escape root"
						);
			}
		}
		else
		{
			legal_segments.push_back(s);
		}
	}
	//@ASSUMPTION: target path always starts with '/'
	std::string cleaned_path = "/";
	for (size_t i = 0; i < legal_segments.size(); ++i)
	{
		cleaned_path += legal_segments[i];
		if (i + 1 < legal_segments.size() || is_dir)
			cleaned_path += "/";
	}

	// find the best matching location 
	std::string remainder;
	std::string location = cleaned_path;
	std::string resolved_path = cleaned_path;
	while (location != "/" && !location.empty())
	{

		size_t pos = location.find_last_of("/");
		location = location.substr(0, pos);
		remainder = cleaned_path.substr(pos);
		if (utils::contains(m_service.locations, location))
		{
			lc = m_service.locations.at(location);
			const std::string& root = lc.root_dir;
			resolved_path = utils::join_paths(root, remainder);
			break;
		}
	}

	return resolved_path;
}

// execute the request depending on the method
void HttpResponse::apply_method(const Path& path, const LocationConfig& location)
{
	if (m_request.method() == "GET")
		apply_GET(path, location);
	else if (m_request.method() == "POST")
		apply_POST(path, location);
	else if (m_request.method() == "DELETE")
		apply_DELETE(path, location);
	else
	{
		throw HttpResponseException(
				StatusCode::NotImplemented, 
				utils::fmt("%s method is not implemented by the server", m_request.method().c_str())
				);
	}
}

void HttpResponse::apply_GET(const Path& path, const LocationConfig& location)
{
	if (!utils::contains(location.methods, "GET"))
	{
		throw HttpResponseException(
				StatusCode::MethodNotAllowed,
				utils::fmt("GET not allowed on this location: '%s'", location.name.c_str()),
				location
				);
	}

	Route redirection = location.redirection.code 
		? location.redirection 
		: m_service.redirection;
	if (redirection.code)
	{
		build_redirection_response(redirection);
	}
	else if (path.is_directory)
	{
		if (!path.can_execute)
		{
			throw HttpResponseException(
					StatusCode::Forbidden, 
					utils::fmt("Can't access directory: '%s'" , path.resolved.c_str()), 
					location
					);
		}
		else if (!path.ends_with_slash)
		{
			throw HttpResponseException(
					StatusCode::MovedPermanently, 
					utils::fmt("%s was moved permanently", path.resolved.c_str()), 
					location
					);
		}
		else if (!location.default_file.empty())
		{
			Path new_path = utils::join_paths(path.resolved, location.default_file);

			if (new_path.exists && new_path.can_read)
			{
				build_response(new_path.resolved);
			}
			else if (!new_path.exists)
			{
				throw HttpResponseException(
						StatusCode::NotFound,
						utils::fmt("'%s' path not found", new_path.resolved.c_str()),
						location
						);
			}
			else if (new_path.is_regular_file && !new_path.can_read)
			{
				throw HttpResponseException(
						StatusCode::Forbidden,
						utils::fmt("Can't read file: '%s'", new_path.resolved.c_str()),
						location);
			}
			else  if (!new_path.is_regular_file)
			{
				throw HttpResponseException(
						StatusCode::Forbidden,
						utils::fmt("Not a regular file: '%s'", new_path.resolved.c_str()),
						location
						);
			}
			else
			{
				throw HttpResponseException(
						StatusCode::InternalServerError,
						utils::fmt("Not valid: '%s'", new_path.resolved.c_str()),
						location
						);
			}
		}
		else if (location.enable_dir_listing)
		{
			build_response(path);
		}
		else
		{
			throw HttpResponseException(
					StatusCode::Forbidden,
					utils::fmt("Directory '%s' has no index and autoindex", path.resolved.c_str()),
					location
					);
		}
	}
	else if (path.is_regular_file)
	{
		if (path.can_read)
		{
			build_response(path);
		}
		else
		{
			throw HttpResponseException(
					StatusCode::Forbidden,
					utils::fmt("Can't read file '%s", path.resolved.c_str()),
					location
					);
		}
	}
	else 
	{
		throw HttpResponseException(
				StatusCode::InternalServerError,
				utils::fmt("Uknown file type '%s", path.resolved.c_str()),
				location
				);
	}
}

void HttpResponse::apply_POST(const Path& path, const LocationConfig& location)
{
	if (!utils::contains(location.methods, "POST"))
	{
		throw HttpResponseException(
				StatusCode::MethodNotAllowed,
				utils::fmt("POST not allowed on this location: '%s'", location.name.c_str()),
				location
				);
	}

	Route redirection = location.redirection.code 
		? location.redirection 
		: m_service.redirection;
	if (redirection.code)
	{
		build_redirection_response(redirection);
	}
	else
	{
		if (path.is_cgi)
		{
			throw std::runtime_error("@TODO: cgi()");
		}
		else if (location.enable_upload_files) 
		{
			Path uploaded = upload_file(location);
			build_post_response(uploaded);
		}
		else
		{
			throw HttpResponseException(
					StatusCode::BadRequest,
					utils::fmt("'%s' is neither cgi nor a regular file", path.resolved.c_str()),
					location
					);
		}
	}
}

Path HttpResponse::upload_file(const LocationConfig& lc)
{
	static unsigned long long uploaded_file_index;

	if (lc.upload_dir.empty())
		throw HttpResponseException(
				StatusCode::InternalServerError,
				utils::fmt("Can't upload file: location '%s' is missing upload directory path", lc.name.c_str()),
				lc

				);
	if ((lc.max_body_size && m_request.body().size() > lc.max_body_size) || 
			(m_request.body().size() && m_request.body().size() > m_service.max_body_size) || 
			m_request.body().size() > constants::max_body_size)
	{
		throw HttpResponseException(
				StatusCode::ContentTooLarge,
				utils::fmt("Request body size is larger than expected"),
				lc
				);
	}

	Path upload_dir(lc.upload_dir);
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

	// create a name for the new uploaded file
	std::string file_name = utils::to_string(uploaded_file_index++) + ".data";
	std::string upload_path = utils::join_paths(lc.upload_dir, file_name);

	// write to file
	std::ofstream file;
	file.open(upload_path.c_str());
	if (!file.is_open())
	{
		throw HttpResponseException(
				StatusCode::InternalServerError,
				utils::fmt("Failed when creating uploaded file: '%s'", upload_path.c_str())
				);
	}
	file << m_request.body();

	return Path(upload_path);
}

void HttpResponse::apply_DELETE(const Path& path, const LocationConfig& location)
{
	if (!utils::contains(location.methods, "DELETE"))
	{
		throw HttpResponseException(
				StatusCode::MethodNotAllowed,
				utils::fmt("DELETE not allowed on this location: '%s'", location.name.c_str()),
				location
				);
	}

	Route redirection = location.redirection.code 
		? location.redirection 
		: m_service.redirection;
	if (redirection.code)
	{
		build_redirection_response(redirection);
	}

	if (path.is_directory)
	{
		throw HttpResponseException(
				StatusCode::Conflict,
				utils::fmt("Cannot delete directory: '%s", path.resolved.c_str()),
				location
				);
	}

	if (std::remove(path.resolved.c_str()) != 0)
	{
		throw HttpResponseException(
				StatusCode::InternalServerError,
				utils::fmt("Failed to delete file: '%s", path.resolved.c_str()),
				location
				);
	}
	build_delete_response();
}

void HttpResponse::build_delete_response()
{
	// status
	write_status_line(StatusCode::NoContent);
	// headers
	m_headers["Connection"] = "close"; // @NOTE: HTTP1.0 closes by default
	m_headers["Date"] = http_date();
}

std::ostream& operator<<(std::ostream& os, const HttpResponse& response)
{
	os << "Response:\n";
	os << response.status_line_str();
	os << response.headers_str();
	if (response.body_type() == HttpResponse::BodyType::String)
	{
		os << response.body_str();
	}
	else if (response.body_type() == HttpResponse::BodyType::File)
		os << response.body_file_path().resolved;
	else if (response.body_type() == HttpResponse::BodyType::Cgi)
		os << "@TODO: implement cgi";
	os << "\n";
	return os;
}

