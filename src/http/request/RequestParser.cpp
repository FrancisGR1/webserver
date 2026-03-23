#include <cstdlib>

#include "core/utils.hpp"
#include "core/constants.hpp"
#include "core/Logger.hpp"
#include "http/StatusCode.hpp"
#include "http/request/Request.hpp"
#include "http/request/RequestParser.hpp"

RequestParser::RequestParser()
	: m_status_code(StatusCode::Ok)
	, m_state(Parser::StartLineMethod)
	, m_idx(0)
	, m_ch('\0')
	, m_content_length(0)
	, m_chunk_size(0)
	, m_done(false)
{}

void RequestParser::feed(const char* raw)
{
	m_buffer += raw;
	parse();
}

void RequestParser::feed(char c)
{
	m_buffer += c;
	parse();
}

bool RequestParser::done() const
{
	return m_state == Parser::Done;
}

Request RequestParser::get() const
{
	return Request(m_method,
			m_target_path,
			m_target_query,
			m_protocol_version,
			m_headers,
			m_body,
			m_status_code);
}

bool RequestParser::error() const
{
	return (m_state == Parser::Error);
}

void RequestParser::clear()
{
	m_method.clear();
	m_target_path.clear();
	m_target_query.clear();
	m_protocol_version.clear();
	m_headers.clear();
	m_body.clear();
	m_status_code = StatusCode::Ok;

	m_state = Parser::StartLineMethod;
	m_idx = 0;
	m_ch = '\0';

	m_buffer.clear();
	m_header_key.clear();
	m_header_value.clear();

	m_content_length = 0;
	m_chunk_size_str.clear();
	m_chunk_size = 0;

	m_done = false;
}

// private 
void RequestParser::parse()
{
	typedef Parser S; // S = state
	for (; m_state != S::Error && m_state != S::Done && m_idx < m_buffer.size(); m_idx++)
	{
		m_ch = m_buffer.at(m_idx);
		switch (m_state)
		{
			// Start Line
			case S::StartLineMethod:
				if (m_ch == ' ') 
				{
					//@TODO: substituir por um constant set
					if (m_method != "GET" && m_method != "POST" && m_method != "DELETE")
					{
						Logger::error("Http Parser: invalid method: %s", m_method.c_str());
						m_state = S::Error;
						m_status_code = StatusCode::NotImplemented;
					}
					else
					{
						m_state = S::StartLineTargetPath;
					}
				}
				else 
				{
					m_method += m_ch;
				}
				break ;
			case S::StartLineTargetPath:
				if (m_ch == ' ')
				{
					if (m_target_path.empty() || m_target_path.at(0) != '/')
					{
						m_state = S::Error;
						m_status_code = StatusCode::BadRequest;
						Logger::error("Http Parser: invalid target: %s", m_target_path.c_str());
					}
					else
					{
						m_state = S::StartLineProtocolVersion;
					}
				}
				else if (m_ch == '?')
				{
					m_state = S::StartLineTargetQuery;
				}
				else
				{
					if (m_target_path.size() > constants::max_uri_size)
					{
						m_state = S::Error;
						m_status_code = StatusCode::UriTooLong;
						Logger::error("Http Parser: target path too large");
					}
					else
					{
						m_target_path += m_ch;
					}
				}
				break ;
			case S::StartLineTargetQuery:
				if (m_ch == ' ')
				{
					m_state = S::StartLineProtocolVersion;
				}
				else
				{
					if (m_target_path.size() + m_target_query.size() > constants::max_uri_size)
					{
						m_state = S::Error;
						m_status_code = StatusCode::UriTooLong;
						Logger::error("Http Parser: target path 'target query too large");
					}
					else
					{
						m_target_query += m_ch;
					}
				}
				break ;
			case S::StartLineProtocolVersion:
				if (m_ch == '\r')
				{
					//@TODO: substituir por um constant set
					std::string pv = m_protocol_version;
					if (pv != "HTTP/1.0" && pv != "HTTP/1.1")
					{
						m_state = S::Error;
						m_status_code = StatusCode::HttpVersionNotSupported;
						Logger::error("Http Parser: invalid version: %s", pv.c_str());
						break;
					}
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
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: expected \\n at end of start line");
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
					if (!is_tchar(m_ch))
					{
						m_state = S::Error;
						m_status_code = StatusCode::BadRequest;
						Logger::error("Http Parser: invalid header key char: '%c'", m_ch);
					}
					else
					{
						m_header_key += m_ch;
					}
				}
				break ;
			case S::HeaderValue:
				if (m_ch == '\r')
				{
					// key is normalized
					std::string k = utils::str_tolower(m_header_key);
					utils::str_trim_sides(k, constants::body_whitespaces);
					if (utils::contains(m_headers, k))
					{
						//@TODO: isto não é necessariamente erro, 
						//alternativa: concatenam-se os valores
						m_state = S::Error;
						m_status_code = StatusCode::BadRequest;
						Logger::error("Http Parser: duplicated header: %s", k.c_str());
						break;
					}
					m_headers[k] = m_header_value;
					m_header_key.clear();
					m_header_value.clear();

					m_state = S::HeaderEndLineCR;
				}
				else
				{
					if (!is_vchar(m_ch) && !is_ows(m_ch))
					{
						m_state = S::Error;
						m_status_code = StatusCode::BadRequest;
						Logger::error("Http Parser: invalid header value char: '%c'", m_ch );
					}
					else
					{
						m_header_value += m_ch;
					}
				}
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
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: expected \\n at end of header line");
				}
				break;
			case S::HeadersEndCR:
				{
					if (m_ch != '\n')
					{
						m_state = S::Error;
						m_status_code = StatusCode::BadRequest;
						Logger::error("Http Parser: expected \\n at end of headers");
						break;
					}

					bool has_content_length = utils::contains(m_headers, "content-length");
					bool has_transfer_encoding = utils::contains(m_headers, "transfer-encoding");
					bool body_is_chunked = false;

					if (has_content_length && has_transfer_encoding)
					{
						m_state = S::Error;
						m_status_code = StatusCode::BadRequest;
						Logger::error("Http Parser: contains incompatible headers: Content-Length and Transfer-Encoding");
						break;
					}

					if (has_transfer_encoding)
					{
						std::string& te = m_headers["transfer-encoding"];
						te = utils::str_tolower(te);
						if (te.find("chunked") != std::string::npos)
							body_is_chunked = true;
						else
						{
							m_state = S::Error;
							m_status_code = StatusCode::NotImplemented;

							Logger::error("Http Parser: %s is not implemented for transfer-enconding", te.c_str());
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
						if (!utils::str_isdigit(value, true))
						{
							m_state = S::Error;
							m_status_code = StatusCode::BadRequest;

							Logger::error("Http Parser: Content-Length doesn't contain a digit: %s", value.c_str());
							break;
						}

						//@TODO: substituir por strtol (melhor para overflows)
						long cl = std::atol(value.c_str());
						if (cl < 0)
						{
							m_state = S::Error; 
							m_status_code = StatusCode::BadRequest;
							Logger::error("Http Parser: Content-Length is negative: %ld", cl);
						}
						else if (cl > static_cast<long>(constants::max_body_size))
						{
							m_state = S::Error;
							m_status_code = StatusCode::ContentTooLarge;
							Logger::error("Http Parser: Content-Length (%ld) exceeds max_body_size(%ld)", cl, constants::max_body_size);
						}
						else if (cl == 0) 
						{
							m_state = S::Done;
						}
						else
						{
							m_state = S::Body;
						}

						m_content_length = static_cast<long unsigned int>(cl);
					}
					else
					{
						m_state = S::Done; /* MODIFICADO */
					// 	m_state = S::Error;
					// 	m_status_code = StatusCode::LengthRequired;
					// Logger::error("Http Parser: doesn't contain neither 'Content-Length' or 'Transfer-Encoding' headers");
					}
					break;
				}
				// Body string
			case S::Body:
				if (m_body.size() == m_content_length)
				{
					m_state = S::Done;
				}
				else if (m_body.size() < m_content_length)
				{
					m_body += m_ch;
					m_state = S::Body;
				}
				else
				{
					m_state = S::Error;
					if (m_content_length == 0)
						m_status_code = StatusCode::LengthRequired;
					else // @NOTE: should never reach here
						m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: body (%ld) is largerthan expected size (%ld)", m_body.size(), m_content_length);
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
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: expected a hexadecimal char, but got this '%c' instead", m_ch);
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
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: expected \\n at end of body chunk size");
				}
				else
				{
					long long hexa = utils::str_tohexadecimal(m_chunk_size_str);
					if (hexa < 0)
					{
						m_state = S::Error;
						m_status_code = StatusCode::BadRequest;
						Logger::error("Http Parser: hexadecimal is negative: %ld", hexa);
						break;
					}
					if (hexa > static_cast<long long>(constants::max_body_size))
					{
						m_state = S::Error;
						m_status_code = StatusCode::ContentTooLarge;
						Logger::error("Http Parser: chunk size (%ld) larger than max_body_size (%ld)", hexa, constants::max_body_size);
						break;
					}
					m_chunk_size = static_cast<size_t>(hexa);
					m_chunk_size_str.clear();
					if (m_chunk_size == 0)  
					{
						//@TODO: testar
						m_state = S::BodyChunkTrailerKey;
					}
					else 			
					{
						m_state = S::BodyChunkData;
					}
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

					if (m_body.size() > constants::max_body_size)
					{
						m_state = S::Error;
						m_status_code = StatusCode::ContentTooLarge;
						Logger::error("Http Parser: body chunk data (%ld) larger than max_body_size (%ld)", m_body.size(), constants::max_body_size);
					}
				}
				else
				{
					m_state = S::Error;
					m_status_code = StatusCode::ContentTooLarge;
					Logger::error("Http Parser: body chunk larger than expected");
				}
				break;
			case S::BodyChunkDataCR:
				if (m_ch != '\n')
				{
					m_state = S::Error;
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: expected \\n at end of body chunk data section");
				}
				else
				{
					m_state = S::BodyChunkSize;
					m_chunk_size = 0;
				}
				break;
			case S::BodyChunkTrailerKey:
				if (m_ch == '\r')
				{
					if (m_header_key.size() == 0)
					{
						m_state = S::BodyChunkTrailerEndCR;
					}
					else
					{
						m_state = S::BodyChunkTrailerCR;
					}
				}
				else if (m_ch == ':')
				{
					m_state = S::BodyChunkTrailerValue;
				}
				else if (!is_tchar(m_ch))
				{
					m_state = S::Error;
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: invalid trailer header key char: '%c'", m_ch);
				}
				else
				{
					m_header_key += m_ch;
				}
				break;
			case S::BodyChunkTrailerValue:
				if (m_ch == '\r')
				{
					// key is normalized
					std::string k = utils::str_tolower(m_header_key);
					utils::str_trim_sides(k, constants::body_whitespaces);
					if (utils::contains(m_headers, k))
					{
						m_state = S::Error;
						m_status_code = StatusCode::BadRequest;
						Logger::error("Http Parser: duplicated trailer header: %s", k.c_str());
						break;
					}
					m_headers[k] = m_header_value;
					m_header_key.clear();
					m_header_value.clear();
					m_state = S::BodyChunkTrailerCR;

				}
				else if (is_vchar(m_ch) || is_ows(m_ch))
				{
					m_header_value += m_ch;
				}
				else
				{
					m_state = S::Error;
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: invalid trailer value char: '%c'", m_ch);
				}
				break;
			case S::BodyChunkTrailerCR:
				if (m_ch != '\n')
				{
					m_state = S::Error;
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: expected \\n at end of body chunk trailer");
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
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: expected \\n at end of body chunk trailer section");
				}
				else
				{
					m_state = S::Done;
				}
				break;
			default:
				{
					m_state = S::Error;
					m_status_code = StatusCode::BadRequest;
					Logger::error("Http Parser: error at default fallthrough");
				}
		}
	}
}

// https://www.rfc-editor.org/rfc/rfc9110.html#name-field-names 
// see 5.6.2
bool RequestParser::is_tchar(char c)
{
	return (std::isalnum(static_cast<unsigned char>(c)) ||
			c == '!' || c == '#' || c == '$' ||
			c == '%' || c == '&' || c == '\'' ||
			c == '*' || c == '+' || c == '-' ||
			c == '.' || c == '^' || c == '_' ||
			c == '`' || c == '|' || c == '~');
}

bool RequestParser::is_vchar(char c)
{
	unsigned char uc = static_cast<unsigned char>(c);
	return (uc >= '!' && uc <= '~');
}

bool RequestParser::is_ows(char c)
{
	return c == ' ' || c == '\t';
}
