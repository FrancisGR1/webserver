#ifndef REQUESTCONFIG_HPP
# define REQUESTCONFIG_HPP

#include <string>

#include "core/Path.hpp"
#include "config/types/LocationConfig.hpp"
#include "config/types/ServiceConfig.hpp"
#include "config/ConfigTypes.hpp"

class HttpRequestConfig
{
	public:
		HttpRequestConfig(const ServiceConfig& service, const Path& path);
		HttpRequestConfig(const ServiceConfig& service, const LocationConfig& location, const Path& path);

		const ServiceConfig& service() const;
		const Path& path() const;
		bool has_location() const;
		const LocationConfig* location() const;
		const Route& redirection() const;

		bool is_cgi() const;
		bool is_redirected() const;
		bool allows_method(const std::string& method) const;
		bool allows_autoindex() const;
		bool has_index() const;
		bool allows_upload() const;
		bool has_upload_dir() const;

		Path index() const;
		Path upload_dir() const;
		Path get_error_page(size_t code) const;
		Path cgi_interpreter() const;
		//@TODO: get listener (a connection é que tem de dar isto)
		size_t max_body_size() const;


	private:
		const ServiceConfig& m_service;
		const Path& m_server_path;
		const LocationConfig* m_location;
};

#endif // REQUESTCONFIG_HPP
