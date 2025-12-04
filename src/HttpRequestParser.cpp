#include <string>
#include <map>
#include <cstdlib>
#include "utils.hpp"
#include "constants.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"

HttpRequestParser::HttpRequestParser()
	: m_state(Parser::StartLineMethod)
	, m_idx(0)
	, m_ch('\0')
	, m_error_code(0)
	, m_content_length(0)
	, m_chunk_size(0)
	, m_done(false)
{}
	  

void HttpRequestParser::feed(const char* raw)
{
	m_buffer += raw;
	parse();
}

void HttpRequestParser::feed(char c)
{
	m_buffer += c;
	parse();
}

bool HttpRequestParser::done() const
{
	return m_state == Parser::Done;
}

bool HttpRequestParser::error() const
{
	return m_state == Parser::Error;
}

HttpRequest HttpRequestParser::get() const
{
	return HttpRequest(m_method,
			m_target_path,
			m_target_query,
			m_protocol_version,
			m_headers,
			m_body);
}


void HttpRequestParser::clear()
{
	m_method.clear();
	m_target_path.clear();
	m_target_query.clear();
	m_protocol_version.clear();
	m_headers.clear();
	m_body.clear();

	m_state = Parser::StartLineMethod;
	m_idx = 0;
	m_ch = '\0';
	m_error.clear();
	m_error_code = 0;

	m_buffer.clear();
	m_header_key.clear();
	m_header_value.clear();

	m_content_length = 0;
	m_chunk_size_str.clear();
	m_chunk_size = 0;

	m_done = false;
}

// private 
void HttpRequestParser::parse()
{
	typedef Parser S; // S = state
	for (; m_state != S::Error && m_idx < m_buffer.size(); m_idx++)
	{
		m_ch = m_buffer.at(m_idx);
		switch (m_state)
		{
			// Start Line
			case S::StartLineMethod:
				if (m_ch == ' ') 
				{
					m_state = S::StartLineTargetPath;
				}
				else 
				{
					m_method += m_ch;
				}
				break ;
			case S::StartLineTargetPath:
				if (m_ch == ' ')
				{
					m_state = S::StartLineProtocolVersion;
				}
				else if (m_ch == '?')
				{
					m_state = S::StartLineTargetQuery;
				}
				else
				{
					m_target_path += m_ch;
				}
				break ;
			case S::StartLineTargetQuery:
				if (m_ch == ' ')
				{
					m_state = S::StartLineProtocolVersion;
				}
				else
				{
					m_target_query += m_ch;
				}
				break ;
			case S::StartLineProtocolVersion:
				if (m_ch == '\r')
				{
					m_state = S::StartLineCR;
				}
				else
				{
					m_protocol_version += m_ch;
				}
				break ;
				// Start Line - End
			case S::StartLineCR:
				if (m_ch != '\n')
				{
					m_state = S::Error;
				}
				else
				{
					m_state = S::HeaderKey;
				}
				break ;
				// Header
			case S::HeaderKey:
				if (m_ch == '\r')
				{
					m_state = S::HeadersEndCR;
				}
				else if (m_ch == ':')
				{
					m_state = S::HeaderValue;
				}
				else
				{
					m_header_key += m_ch;
				}
				break ;
			case S::HeaderValue:
				if (m_ch == '\r')
				{
					utils::str_tolower(m_header_key);
					utils::str_trim_sides(m_header_value, constants::BODY_WHITESPACES);
					if (utils::map_contains(m_headers, m_header_key))
					{
						m_state = S::Error;
						break;
					}
					m_headers[m_header_key] = m_header_value;
					m_header_key.clear();
					m_header_value.clear();
					m_state = S::HeaderEndLineCR;
				}
				else
					m_header_value += m_ch;
				break;
				// Header Line - End
			case S::HeaderEndLineCR:
				if (m_ch == '\n')
				{
					m_state = S::HeaderKey;
				}
				else
				{
					m_state = S::Error;
				}
				break;
			case S::HeadersEndCR:
				{
					if (m_ch != '\n')
					{
						m_state = S::Error;
						break;
					}

					bool has_content_length = utils::map_contains(m_headers, "content-length");
					bool has_transfer_encoding = utils::map_contains(m_headers, "transfer-encoding");
					bool body_is_chunked = false;

					if (has_transfer_encoding)
					{
						std::string& te = m_headers["transfer-encoding"];
						utils::str_tolower(te);
						if (te.find("chunked") != std::string::npos)
							body_is_chunked = true;
						else
						{
							m_state = S::Error;
							break ;
						}

					}

					if (body_is_chunked)
					{
						m_state = S::BodyChunkSize;
					}
					else if (has_content_length)
					{
						const std::string& value = m_headers["content-length"];
						if (!utils::str_isdigit(value))
						{
							m_state = S::Error;
							break;
						}

						long cl = std::atol(value.c_str());
						//@TODO: verificar que não excede o max_body_size
						if (cl < 0)       m_state = S::Error;
						else if (cl == 0) m_state = S::Done;
						else              m_state = S::Body;

						m_content_length = static_cast<long unsigned int>(cl);
					}
					else
					{
						m_state = S::Body;
					}
					break;
				}
				// Body string
			case S::Body:
				if (m_body.size() == m_content_length && m_ch == '\r')
				{
					m_state = S::BodyCR;
				}
				else if (m_body.size() < m_content_length)
				{
					m_body += m_ch;
					m_state = S::Body;
				}
				else
				{
					m_state = S::Error;
				}
				break;
			case S::BodyCR:
				if (m_ch != '\n')
				{
					m_state = S::Error;
				}
				else
				{
					m_state = S::Done;
				}
				break;
				// Body chunked
			case S::BodyChunkSize:
				if (m_ch == '\r')
				{
					m_state = S::BodyChunkSizeCR;
				}
				else if (m_ch == ';')
				{
					m_state = S::BodyChunkSizeExtension;
				}
				else if (std::isxdigit(m_ch))
				{
					m_chunk_size_str += m_ch;
				}
				else
				{
					m_state = S::Error;
				}
				break ;
			case S::BodyChunkSizeExtension:
				if (m_ch == '\r')
				{
					m_state = S::BodyChunkSizeCR;
				}
				else
				{
					//ignore chunk extensions
				}
				break;
			case S::BodyChunkSizeCR:
				if (m_ch != '\n')
				{
					m_state = S::Error;
				}
				else
				{
					int hexa = utils::str_tohexadecimal(m_chunk_size_str);
					if (hexa < 0)
					{
						m_state = S::Error;
						break;
					}
					m_chunk_size = static_cast<size_t>(hexa);
					m_chunk_size_str.clear();
					if (m_chunk_size == 0)  
						m_state = S::BodyChunkTrailerKey;
					else 			
						m_state = S::BodyChunkData;
				}
				break;
			case S::BodyChunkData:
				if (m_chunk_size == 0 && m_ch == '\r')
				{
					m_state = S::BodyChunkDataCR;
				}
				else if (m_chunk_size > 0)
				{
					m_body += m_ch;
					m_chunk_size--;
				}
				else
				{
					m_state = S::Error;
				}
				break;
			case S::BodyChunkDataCR:
				if (m_ch != '\n')
				{
					m_state = S::Error;
				}
				else
				{
					m_state = S::BodyChunkSize;
					m_chunk_size = 0;
				}
				break;
			case S:: BodyChunkTrailerKey:
				if (m_ch == '\r')
				{
					m_state = S::BodyChunkTrailerEndCR;
				}
				else if (m_ch == ':')
				{
					m_state = S::BodyChunkTrailerValue;
				}
				else
				{
					m_header_key += m_ch;
				}
				break;
			case S::BodyChunkTrailerValue:
				if (m_ch == '\r')
				{
					//@TODO: fazer uma func que faça isto abaixo
					utils::str_tolower(m_header_key);
					utils::str_trim_sides(m_header_value, constants::BODY_WHITESPACES);
					if (utils::map_contains(m_headers, m_header_key))
					{
						m_state = S::Error;
						break;
					}
					m_headers[m_header_key] = m_header_value;
					m_header_key.clear();
					m_header_value.clear();
					m_state = S::BodyChunkTrailerCR;
				}
				else
				{
					m_header_value += m_ch;
				}
				break;
			case S::BodyChunkTrailerCR:
				if (m_ch != '\n')
				{
					m_state = S::Error;
				}
				else
				{
					m_state = S::BodyChunkTrailerKey;
				}
				break;
			case S::BodyChunkTrailerEndCR:
				if (m_ch != '\n')
				{
					m_state = S::Error;
				}
				else
				{
					m_state = S::Done;
				}
				break;
			default:
				m_state = S::Error;
		}
	}
}
