#ifndef NEWHTTPRESPONSE_HPP
# define NEWHTTPRESPONSE_HPP

#include <map>
#include <string>

#include "StatusCode.hpp"
#include "IBody.hpp"

class NewHttpResponse
{
	public:
		explicit NewHttpResponse(StatusCode::Code status);
		~NewHttpResponse();

		void set_header(const std::string& key, const std::string& value);
		void set_body(IBody* body);
		StatusCode::Code status() const;
		bool send(int fd) const;
		bool done() const;

	private:
		const StatusCode::Code m_status;
		std::map<std::string, std::string> m_headers;
		IBody* m_body;

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


		// no copy semantics, because no clone()
		NewHttpResponse(const NewHttpResponse& other);
		NewHttpResponse& operator=(const NewHttpResponse& other);

};


# endif // NEWHTTPRESPONSE_HPP
