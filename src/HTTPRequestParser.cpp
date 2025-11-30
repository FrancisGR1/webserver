#include <string>
#include <map>
#include <cstdlib>
#include "utils.hpp"
#include "HTTPRequest.hpp"
#include "HTTPRequestParser.hpp"


//Debugging:
//#include <iostream>


HTTPRequestParser::HTTPRequestParser()
	: m_state(Parser::State::StartLineMethod)
	, m_idx(0)
	, m_ch('\0')
	, m_error_code(0)
	, m_done(false)
{}
	  

void HTTPRequestParser::feed(const char* raw)
{
	m_buffer += raw;

	typedef Parser::State S;
	for (; m_state != S::Error && m_idx < m_buffer.size(); m_idx++)
	{
		m_ch = m_buffer.at(m_idx);
		switch (m_state)
		{
			// Start Line
			case S::StartLineMethod:
				if (m_ch == ' ') 
					m_state = S::StartLineTarget;
				else 
					m_method += m_ch;
				break ;
			case S::StartLineTarget:
				if (m_ch == ' ')
					m_state = S::StartLineProtocolVersion;
				else
					m_target += m_ch;
				break ;
			case S::StartLineProtocolVersion:
				if (m_ch == '\r')
					m_state = S::StartLineCR;
				else
					m_protocol_version += m_ch;
				break ;
			// Start Line - End
			case S::StartLineCR:
				if (m_ch != '\n')
					m_state = S::Error;
				else
					m_state = S::HeaderKey;
				break ;
			// Header
			case S::HeaderKey:
				if (m_ch == '\r')
					m_state = S::HeadersEndCR;
				else if (m_ch == ':')
					m_state = S::HeaderValue;
				else
					m_header_key += m_ch;
				break ;
			case S::HeaderValue:
				if (m_ch == '\r')
				{
					Utils::str_tolower(m_header_key);
					Utils::str_trim_sides(m_header_value, " \t\n\f\v");
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
					m_state = S::HeaderKey;
				else
					m_state = S::Error;
				break;
			case S::HeadersEndCR:
				{
					if (m_ch != '\n')
					{
						m_state = S::Error;
						break;
					}

					bool has_content_length = Utils::map_contains(m_headers, "content-length");
					bool has_transfer_encoding = Utils::map_contains(m_headers, "transfer-encoding");
					bool body_is_chunked = false;

					if (has_transfer_encoding)
					{
						std::string& te = m_headers["transfer-encoding"];
						Utils::str_tolower(te);
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
						if (!Utils::str_isdigit(value))
						{
							m_state = S::Error;
							break;
						}

						long cl = std::atol(value.c_str());
						if (cl < 0)       m_state = S::Error;
						else if (cl == 0) m_state = S::Done;
						else              m_state = S::Body;

						m_content_length = cl;
					}
					else
					{
						m_state = S::Body;
					}
					break;
				}
			case S::Body:
				if (m_ch == '\r')
				{
					if (m_body.size() >= m_content_length)
					{
						m_state = S::Error;
						break ;
					}
					m_state = S::BodyCR;
				}
				else
				{
					m_body += m_ch;
					m_state = S::Body;
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
		}
	}
}

HTTPRequest HTTPRequestParser::get() const
{
	return HTTPRequest(m_method,
			m_target,
			m_protocol_version,
			m_headers,
			m_body);
}
