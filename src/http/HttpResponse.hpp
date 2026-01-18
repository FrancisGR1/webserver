#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <sys/types.h>

#include "core/Path.hpp"
#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"
#include "HttpRequest.hpp"

class HttpResponse
{
	public:
		HttpResponse(const HttpRequest& request, const ServiceConfig& service);
		const std::string& get() const;

	private:
		const ServiceConfig& m_service;
		const HttpRequest& m_request;
		std::string m_response;
		StatusCode::Code m_status_code;
		std::string m_status_line;
		std::map<std::string, std::string> m_headers;
		std::string m_body;

		void apply_method(const Path& path, const LocationConfig& location);

		// GET
		void apply_GET(const Path& path, const LocationConfig& location);
		void build_response(const Path& path);
		void build_redirection_response(const Route& redirection);
		void write_body(const Path& path);
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

		class HttpResponseException : public std::exception
	{
		public:
			explicit HttpResponseException(StatusCode::Code code, const std::string& msg);
			explicit HttpResponseException(StatusCode::Code code, const std::string& msg, const LocationConfig& lc);
			StatusCode::Code status() const;
			const LocationConfig& location() const;
			const std::string& msg() const;

			virtual ~HttpResponseException() throw();

		private:
			StatusCode::Code m_status;
			LocationConfig m_location;
			std::string m_msg;

			//illegal
			HttpResponseException();
	};

};

std::ostream& operator<<(std::ostream& os, const HttpResponse& response);

#endif // HTTPRESPONSE_HPP
