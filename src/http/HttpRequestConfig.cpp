#include "core/constants.hpp"
#include "core/utils.hpp"
#include "core/Path.hpp"
#include "config/types/LocationConfig.hpp"
#include "config/types/ServiceConfig.hpp"
#include "config/ConfigTypes.hpp"
#include "HttpRequestConfig.hpp"

HttpRequestConfig::HttpRequestConfig(const ServiceConfig& service)
	: m_service(service)
	, m_resolved_path(NULL)
	, m_location(NULL) {}

HttpRequestConfig::HttpRequestConfig(const ServiceConfig& service, const Path& path, const LocationConfig& location)
	: m_service(service)
	, m_resolved_path(path)
	, m_location(&location) {}

void HttpRequestConfig::set(const Path& path)
{
	m_resolved_path = path;
}

void HttpRequestConfig::set(const LocationConfig& location)
{
	m_location = &location;
};

const ServiceConfig& HttpRequestConfig::service() const { return  m_service; }

const Path& HttpRequestConfig::path() const { return  m_resolved_path; }

bool HttpRequestConfig::has_location() const { return  m_location != NULL; }

const LocationConfig* HttpRequestConfig::location() const { return  m_location; }

const Route& HttpRequestConfig::redirection() const
{
	if (m_location) 
		return m_location->redirection;
	else
		return m_service.redirection;
}

bool HttpRequestConfig::is_cgi() const { return m_resolved_path.is_cgi; }


bool HttpRequestConfig::is_redirected() const
{  
	if (m_location)
		return StatusCode::is_redirection(m_location->redirection.code);
	return StatusCode::is_redirection(m_service.redirection.code);
}

bool HttpRequestConfig::allows_method(const std::string& method) const
{
	if (m_location) return utils::contains(m_location->methods, method);
	return (false);
}

bool HttpRequestConfig::allows_autoindex() const
{
	if (m_location) 
		return m_location->enable_dir_listing;
	return (false);
}

bool HttpRequestConfig::has_index() const
{
	if (m_location) 
		return !m_location->default_file.empty();
	return false;
}

bool HttpRequestConfig::allows_upload() const
{
	if (m_location) 
		return m_location->enable_upload_files;
	return (false);
}

bool HttpRequestConfig::has_upload_dir() const
{
	if (m_location) 
		return !m_location->upload_dir.empty();
	return false;
}

Path HttpRequestConfig::index() const
{
	if (m_location)
		return m_location->default_file;
	return Path(NULL);
}

Path HttpRequestConfig::upload_dir() const
{
	if (m_location)
		return m_location->upload_dir;
	return Path(NULL);
}

Path HttpRequestConfig::get_error_page_or_nonexistent_path(size_t code) const
{
	if (m_location && utils::contains(m_location->error_pages, code))
		return m_location->error_pages.at(code);
	else if (utils::contains(m_location->error_pages, code))
		return m_service.error_pages.at(code);
	else 
		return Path(NULL); //@TODO construir string?
}

Path HttpRequestConfig::cgi_interpreter() const
{
	const std::string& extension = m_resolved_path.cgi_extension;

	if (m_location && utils::contains(m_location->cgis, extension))
		return m_location->cgis.at(extension);

	return Path(NULL);
}

//@TODO: get listener (a connection é que tem de dar isto)
size_t HttpRequestConfig::max_body_size() const
{
	if (m_location->max_body_size)
		return m_location->max_body_size;
	else if (m_service.max_body_size)
		return m_service.max_body_size;
	else
		return constants::max_body_size;
}
