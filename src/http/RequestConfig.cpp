#include "core/constants.hpp"
#include "core/utils.hpp"
#include "core/Path.hpp"
#include "config/types/LocationConfig.hpp"
#include "config/ConfigTypes.hpp"
#include "RequestConfig.hpp"


HttpRequestConfig::HttpRequestConfig(const HttpRequest& request, const ServiceConfig& service, const Path& path)
	: m_service(service)
	, m_server_path(path)
	, m_location(NULL) {}

HttpRequestConfig::HttpRequestConfig(const HttpRequest& request, const ServiceConfig& service, const LocationConfig& location, const Path& path)
	: m_service(service)
	, m_server_path(path)
	, m_location(&location) {}

const Route& HttpRequestConfig::redirection() const
{
	if (m_location) 
		return m_location.redirection;
	else
		return m_service.location;
}

const Path& HttpRequestConfig::path() const
{
	return m_server_path;
}

bool HttpRequestConfig::allows_method(const std::string& method) const
{
	if (m_location) return utils::contains(m_location.methods, method);
	return (false);
}

bool HttpRequestConfig::allows_autoindex() const
{
	if (m_location) 
		return m_location.enable_dir_listing;
	return (false);
}

bool HttpRequestConfig::allows_upload() const
{
	if (m_location) 
		return m_location.enable_upload;
	return (false);
}

bool HttpRequestConfig::has_upload_dir() const
{
	if (m_location) 
		return !m_location.upload_dir.empty();
	return false;
}

Path HttpRequestConfig::get_error_page(size_t code) const
{
	if (m_location && utils::contains(m_location.error_pages, code))
		return m_location.error_pages[code];
	else 
		return m_service.error_pages[code];
}

Path HttpRequestConfig::get_cgi_path(const std::string& extension)
{
	if (m_location && utils::contains(m_location.cgis, extension))
		return m_location.error_pages[extension];
	return Path(NULL);
}

//@TODO: get listener (a connection é que tem de dar isto)
size_t HttpRequestConfig::max_body_size() const
{
	if (m_location.max_body_size)
		return m_location.max_body_size;
	else if (m_service.max_body_size)
		return m_service.max_body_size;
	else
		return constants::max_body_size;
}
