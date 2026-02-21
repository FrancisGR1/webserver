#ifndef NEWHTTPRESPONSE_HPP
# define NEWHTTPRESPONSE_HPP

#include <map>
#include <string>
#include <sys/types.h>

#include "core/Path.hpp"
#include "StatusCode.hpp"

class NewHttpResponse
{
	public:
		NewHttpResponse();
		explicit NewHttpResponse(StatusCode::Code status);
		~NewHttpResponse();

		void set_status(StatusCode::Code status);
		void set_header(const std::string& key, const std::string& value);
		void set_body_as_str(const std::string& str);
		void set_body_as_fd(int fd, const std::string& prefix = "");
		void set_body_as_path(const Path& path);
		StatusCode::Code status() const;
		ssize_t send(int fd);
		bool done() const;

	private:
		StatusCode::Code m_status;
		std::string m_status_line;
		std::string m_headers_str;
		std::map<std::string, std::string> m_headers;

		// body is either one of the following
		std::string m_body_str;
		int m_body_fd;

		// transmission state
		enum SendPhase
		{
			StatusPhase = 0,
			HeadersPhase,
			BodyPhase,
			Done
		};

		SendPhase m_send_phase;
		size_t m_offset;

		// utils
		std::string make_status_line();
		void handle_file(NewHttpResponse& response, const Path& path);


		// no copy semantics, because no clone()
		NewHttpResponse(const NewHttpResponse& other);
		NewHttpResponse& operator=(const NewHttpResponse& other);

};


# endif // NEWHTTPRESPONSE_HPP
