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
		StartLineTargetPath,
		StartLineTargetQuery,
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
		BodyChunkSize,
		BodyChunkSizeExtension,
		BodyChunkSizeCR,
		BodyChunkData,
		BodyChunkDataCR,
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
		bool done() const;
		bool error() const;
		HTTPRequest get() const;
		void clear();

	private:
		// request for get()
		HTTPRequest request;
		// HTTP elements
		std::string m_method;
		std::string m_target_path;
		std::string m_target_query;
		std::string m_protocol_version;
		std::map<std::string, std::string> m_headers;
		std::string m_body;


		// info for the moment of parsing
		Parser::State m_state;
		size_t m_idx;
		unsigned char m_ch;
		std::string m_error;
		long m_error_code;
		// store temporary values
		std::string m_buffer;
		std::string m_header_key;
		std::string m_header_value;
		// body data
		long m_content_length;
		std::string m_chunk_size_str;
		size_t m_chunk_size;

		// is parsing over?
		bool m_done;
};

#endif //HTTPREQUESTPARSER_HPP
