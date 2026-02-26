#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

#include <string>
#include <map>

#include "http/StatusCode.hpp"
#include "http/request/Request.hpp"

// @TODO: mudar para outro ficheiro
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


class RequestParser
{
	public:
		RequestParser();

		void feed(const char* raw);
		void feed(char c);
		bool done() const;
		Request get() const;
		bool error() const;
		void clear();

	private:
		// Http elements
		std::string m_method;
		std::string m_target_path;
		std::string m_target_query;
		std::string m_protocol_version;
		std::map<std::string, std::string> m_headers;
		std::string m_body;
		StatusCode::Code m_status_code;


		// info for the moment of parsing
		Parser::State m_state;
		size_t m_idx;
		unsigned char m_ch;
		// store temporary values
		std::string m_buffer;
		std::string m_header_key;
		std::string m_header_value;
		// body data
		long unsigned int m_content_length;
		std::string m_chunk_size_str;
		size_t m_chunk_size;

		// is parsing over?
		bool m_done;

		void parse();
		
		// header specific char validation
		bool is_tchar(char c);
		bool is_vchar(char c);
		bool is_ows(char c);
};

#endif // REQUESTPARSER_HPP
