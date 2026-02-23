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
		HttpRequestConfig(const ServiceConfig& service);
		//HttpRequestConfig(const ServiceConfig& service, const Path& path);
		//HttpRequestConfig(const ServiceConfig& service, const LocationConfig& location, const Path& path);

		void set_path(const Path& path);
		void set_location(const std::string& resolved_path);

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
		bool has_file_for_error(size_t code) const;

		Path index() const;
		Path upload_dir() const;
		Path get_error_page_or_nonexistent_path(size_t code) const;
		Path cgi_interpreter() const;
		//@TODO: get listener (a connection é que tem de dar isto)
		size_t max_body_size() const;


	private:
		const ServiceConfig& m_service;
		Path m_resolved_path; // transformed request path
		const LocationConfig* m_location;
};

#endif // REQUESTCONFIG_HPP
