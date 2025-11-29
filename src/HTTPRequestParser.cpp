#include <string>
#include <map>
#include "HTTPRequest.hpp"
#include "HTTPRequestParser.hpp"


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

	//parse
	for (; m_state != Parser::State::Error && m_idx < m_buffer.size(); m_idx++)
	{
		m_ch = m_buffer.at(m_idx);
		switch (m_state)
		{
			case (Parser::State::StartLineMethod):
				if (m_ch == ' ') 
					m_state = Parser::State::StartLineTarget;
				else 
					m_method += m_ch;
				break ;
			case (Parser::State::StartLineTarget):
				if (m_ch == ' ')
					m_state = Parser::State::StartLineProtocolVersion;
				else
					m_target += m_ch;
				break ;
			case (Parser::State::StartLineProtocolVersion):
				if (m_ch == ' ')
					m_state = Parser::State::StartLineCR;
				else
					m_protocol_version += m_ch;
				break ;
			case (Parser::State::StartLineCR):
				if (m_ch != '\r')
					m_state = Parser::State::Error;
				else
					m_state = Parser::State::StartLineLF;
				break ;
			case (Parser::State::StartLineLF):
				if (m_ch != '\n')
					m_state = Parser::State::Error;
				else
					m_state = Parser::State::HeaderKey;
				break ;
			//case (Parser::State::HeaderKey):
			//	if (m_ch == ':')
			//		m_state = Parser::State::HeaderValue;
			//	else
			//		m_header_key += m_ch;
			//	break ;
			//case (Parser::State::HeaderValue):
			//	if (m_ch == '\r')
			//		m_state = Parser::State::HeaderEndLineCR;
			//	else
			//		m_header_value += m_ch;
			//	break;
			//case (Parser::State::HeaderEndLineCR):
			//	if (m_ch == '\n')
			//		m_state = Parser::State::HeaderEndLineLF;
			//	else
			//		m_state = Parser::State::Error;
			//	break;
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
