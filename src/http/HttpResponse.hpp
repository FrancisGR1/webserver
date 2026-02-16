#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <sys/types.h>

#include "core/Path.hpp"
#include "config/ConfigTypes.hpp"
#include "config/types/LocationConfig.hpp"
#include "config/types/ServiceConfig.hpp"
#include "StatusCode.hpp"
#include "HttpRequest.hpp"
#include "HttpResponseException.hpp"

class HttpResponse
{
	public:
		// response body type
		struct BodyType
		{
			enum Type
			{
				String = 0,
				File,
				Cgi,
				Non,
			};
		};

		HttpResponse(const HttpRequest& request, const ServiceConfig& service);

		BodyType::Type body_type() const;
		const std::string& status_line_str() const;
		const std::string  headers_str() const;
		const std::string& body_str() const;
		const Path&        body_file_path() const;
		int                body_fd() const;


	private:
		const ServiceConfig& m_service;
		const HttpRequest& m_request;
		std::string m_response; //@TODO: eliminar

		StatusCode::Code m_status_code;
		std::string m_status_line;
		std::map<std::string, std::string> m_headers;

		BodyType::Type m_body_type;
		std::string m_body; // POST, DELETE
		int m_body_fd; // CGI (post)
		Path m_body_file_path; // GET

		void apply_method(const Path& path, const LocationConfig& location);

		// GET
		void apply_GET(const Path& path, const LocationConfig& location);
		void build_response(const Path& path);
		void build_redirection_response(const Route& redirection);
		void set_body(const std::string& string);
		void set_body(const Path& path);
		void write_listing_dir_body(const Path& path);
		void write_headers(const std::string& content_type);


		// POST
		void apply_POST(const Path& path, const LocationConfig& location);
		Path upload_file(const LocationConfig& lc);
		void build_post_response(const Path& uploaded);

		// DELETE
		void apply_DELETE(const Path& path, const LocationConfig& location);
		void build_delete_response();

		// error
		void build_error_response(StatusCode::Code code);
		void build_error_response(StatusCode::Code code, const LocationConfig& lc);

		// utils
		void write_status_line();
		void write_status_line(StatusCode::Code code);
		std::string http_date();
		std::string headers_to_str() const;
		std::string resolved_target(LocationConfig& lc);


};

std::ostream& operator<<(std::ostream& os, const HttpResponse& response);

#endif // HTTPRESPONSE_HPP
