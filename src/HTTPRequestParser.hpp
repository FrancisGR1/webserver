#ifndef HTTPREQUESTPARSER_HPP
# define HTTPREQUESTPARSER_HPP

#include <string>
#include <map>
#include "HTTPRequest.hpp"

struct Parser
{
	enum State
	{
		// === Start line ===
		StartLineMethod,
		StartLineTarget,
		StartLineProtocolVersion,
		StartLineCR,

		// === Headers ===
		HeaderKey,
		HeaderValue,
		HeaderEndLineCR,
		HeadersEndCR,

		// === Body ===
		// Body - full string
		Body,
		BodyCR,
		// Body - chunked
		BodyChunkData,
		BodyChunkDataCR,
		BodyChunkSize,
		BodyChunkSizeCR,
		// Body - trailing headers
		BodyChunkTrailerKey,
		BodyChunkTrailerValue,
		BodyChunkTrailerCR,
		BodyChunkTrailerEndCR,

		// === Final state ===
		Error,
		Done
	};
};


class HTTPRequestParser
{
	public:
		HTTPRequestParser();

		void feed(const char* raw);
		bool is_done() const;
		HTTPRequest get() const;
		void clear();

	private:
		// request for get()
		HTTPRequest request;
		// HTTP elements
		std::string m_method;
		std::string m_target;
		std::string m_protocol_version;
		std::map<std::string, std::string> m_headers;
		std::string m_body;
		long m_content_length;


		// info for the moment of parsing
		Parser::State m_state;
		size_t m_idx;
		char m_ch;
		std::string m_error;
		long m_error_code;
		// store temporary values
		std::string m_buffer;
		std::string m_header_key;
		std::string m_header_value;

		// is parsing over?
		bool m_done;
};

#endif //HTTPREQUESTPARSER_HPP
