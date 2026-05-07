#include <cstdlib>

#include "core/Logger.hpp"
#include "core/constants.hpp"
#include "core/utils.hpp"
#include "http/StatusCode.hpp"
#include "http/request/Request.hpp"
#include "http/request/RequestParser.hpp"

RequestParser::RequestParser()
    : m_status_code(StatusCode::BadRequest)
    , m_state(ParserState::StartLineMethod)
    , m_idx(0)
    , m_ch('\0')
    , m_content_length(0)
    , m_chunk_size(0)
    , m_done(false)
{
}

void RequestParser::feed(const char* raw, size_t len)
{
    Logger::trace("RequestParser: add to buffer: size=%lld", len);

    m_buffer.append(raw, len);

    Logger::debug("RequestParser: new buffer: size=%lld", m_buffer.size());

    parse();
}

bool RequestParser::done() const
{
    return m_state == ParserState::Done || error();
}

ParserState::Enum RequestParser::state() const
{
    return m_state;
}

Request RequestParser::get() const
{
    // clang-format off
    return Request(
		    m_method, 
		    m_target_path, 
		    m_target_query, 
		    m_protocol_version, 
		    m_headers, 
		    m_body, 
		    m_multipart_body,
		    m_status_code
		    );
    // clang-format on
}

bool RequestParser::error() const
{
    return (m_state == ParserState::Error);
}

void RequestParser::clear()
{
    m_method.clear();
    m_target_path.clear();
    m_target_query.clear();
    m_protocol_version.clear();
    m_headers.clear();
    m_body.clear();
    m_status_code = StatusCode::BadRequest;

    m_state = ParserState::StartLineMethod;
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
    typedef ParserState S; // S = state
    Logger::trace("RequestParser: idx=%ld,buffer_size=%ld", m_idx, m_buffer.size());
    for (; m_state != S::Error && m_state != S::Done && m_idx < m_buffer.size(); m_idx++)
    {
        m_ch = m_buffer.at(m_idx);
        switch (m_state)
        {
            // Start Line
            case S::StartLineMethod:
                Logger::verbose("RequestParser: state - Method: '%c'", m_ch);

                if (m_ch == ' ')
                {
                    if (!is_valid_method(m_method))
                    {
                        Logger::error("RequestParser: invalid method: %s", m_method.c_str());
                        m_status_code = StatusCode::BadRequest;
                        m_state = S::Error;
                        break;
                    }
                    if (m_method != "GET" && m_method != "POST" && m_method != "DELETE")
                    {
                        Logger::error("RequestParser: didn't implement method: %s", m_method.c_str());
                        m_status_code = StatusCode::NotImplemented;
                        m_state = S::Error;
                        break;
                    }

                    m_state = S::StartLineTargetPath;
                }
                else
                {
                    m_method += m_ch;
                }
                break;
            case S::StartLineTargetPath:
                Logger::verbose("RequestParser: state - Path: '%c'", m_ch);

                if (m_ch == ' ')
                {
                    if (m_target_path.empty() || m_target_path.at(0) != '/')
                    {
                        m_state = S::Error;
                        m_status_code = StatusCode::BadRequest;
                        Logger::error("RequestParser: invalid target: %s", m_target_path.c_str());
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
                        Logger::error("RequestParser: target path too large");
                    }
                    else
                    {
                        m_target_path += m_ch;
                    }
                }
                break;
            case S::StartLineTargetQuery:
                Logger::verbose("RequestParser: state - Query: '%c'", m_ch);

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
                        Logger::error("RequestParser: target path 'target query too large");
                    }
                    else
                    {
                        m_target_query += m_ch;
                    }
                }
                break;
            case S::StartLineProtocolVersion:
                Logger::verbose("RequestParser: state - Version: '%c'", m_ch);

                if (m_ch == '\r')
                {
                    std::string pv = m_protocol_version;

                    if (!is_http_version(pv))
                    {
                        m_state = S::Error;
                        m_status_code = StatusCode::BadRequest;
                        break;
                    }

                    if (pv != "HTTP/1.0" && pv != "HTTP/1.1")
                    {
                        m_state = S::Error;
                        m_status_code = StatusCode::HttpVersionNotSupported;
                        Logger::error("RequestParser: invalid version: %s", pv.c_str());
                        break;
                    }

                    m_state = S::StartLineCR;
                }
                else
                {
                    m_protocol_version += m_ch;
                }
                break;
                // Start Line - End
            case S::StartLineCR:
                Logger::verbose("RequestParser: state - Status CR: '%c'", m_ch);

                if (m_ch != '\n')
                {
                    m_state = S::Error;
                    m_status_code = StatusCode::BadRequest;
                    Logger::error("RequestParser: expected \\n at end of start line");
                }
                else
                {
                    m_state = S::HeaderKey;
                }
                break;
                // Header
            case S::HeaderKey:
                Logger::verbose("RequestParser: state - Key: '%c'", m_ch);

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
                        Logger::error("RequestParser: invalid header key char: '%c'", m_ch);
                    }
                    else
                    {
                        m_header_key += m_ch;
                    }
                }
                break;
            case S::HeaderValue:
                Logger::verbose("RequestParser: state - Value: '%c'", m_ch);

                if (m_ch == '\r')
                {
                    // key is normalized
                    std::string k = utils::str_tolower(m_header_key);
                    utils::str_trim_sides(k, constants::body_whitespaces);
                    if (utils::contains(m_headers, k))
                    {
                        //@TODO: isto não é necessariamente erro,
                        // alternativa: concatenam-se os valores
                        m_state = S::Error;
                        m_status_code = StatusCode::BadRequest;
                        Logger::error("RequestParser: duplicated header: %s", k.c_str());
                        break;
                    }

                    utils::str_trim_sides(m_header_value, constants::body_whitespaces);
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
                        Logger::error("RequestParser: invalid header value char: '%c'", m_ch);
                    }
                    else
                    {
                        m_header_value += m_ch;
                    }
                }
                break;
                // Header Line - End
            case S::HeaderEndLineCR:
                Logger::verbose("RequestParser: state - Header Line CR: '%c'", m_ch);

                if (m_ch == '\n')
                {
                    m_state = S::HeaderKey;
                }
                else
                {
                    m_state = S::Error;
                    m_status_code = StatusCode::BadRequest;
                    Logger::error("RequestParser: expected \\n at end of header line");
                }
                break;
            case S::HeadersEndCR:
                Logger::verbose("RequestParser: state - Headers CR: '%c'", m_ch);

                {
                    if (m_ch != '\n')
                    {
                        m_state = S::Error;
                        m_status_code = StatusCode::BadRequest;
                        Logger::error("RequestParser: expected \\n at end of headers");
                        break;
                    }

                    bool has_content_length = utils::contains(m_headers, "content-length");
                    bool has_transfer_encoding = utils::contains(m_headers, "transfer-encoding");
                    bool body_is_chunked = false;

                    if (has_content_length && has_transfer_encoding)
                    {
                        m_state = S::Error;
                        m_status_code = StatusCode::BadRequest;
                        Logger::error(
                            "RequestParser: contains incompatible headers: Content-Length and Transfer-Encoding");
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

                            Logger::error("RequestParser: %s is not implemented for transfer-enconding", te.c_str());
                            break;
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

                            Logger::error("RequestParser: Content-Length doesn't contain a digit: %s", value.c_str());
                            break;
                        }

                        //@TODO: substituir por strtol (melhor para overflows)
                        long cl = std::atol(value.c_str());
                        if (cl < 0)
                        {
                            m_state = S::Error;
                            m_status_code = StatusCode::BadRequest;
                            Logger::error("RequestParser: Content-Length is negative: %ld", cl);
                        }
                        else if (cl > static_cast<long>(constants::max_body_size))
                        {
                            m_state = S::Error;
                            m_status_code = StatusCode::ContentTooLarge;
                            Logger::error(
                                "RequestParser: Content-Length (%ld) exceeds max_body_size(%ld)",
                                cl,
                                constants::max_body_size);
                        }
                        else if (cl == 0)
                        {
                            m_state = S::Done;
                            m_status_code = StatusCode::Ok;
                        }
                        else
                        {
                            m_state = S::Body;
                        }

                        m_content_length = static_cast<long unsigned int>(cl);
                        Logger::trace("RequestParser: content-length: %ld", m_content_length);
                    }
                    else
                    {
                        m_state = S::Done; /* MODIFICADO */
                        m_status_code = StatusCode::Ok;
                        // 	m_state = S::Error;
                        // 	m_status_code = StatusCode::LengthRequired;
                        // Logger::error("Http Parser: doesn't contain neither 'Content-Length' or 'Transfer-Encoding'
                        // headers");
                    }
                    break;
                }
                // Body string
            case S::Body:
                Logger::verbose("RequestParser: state - Body(%ld): '%c'", m_body.size() + 1, m_ch);

                if (m_body.size() < m_content_length)
                {
                    size_t bytes_needed = m_content_length - m_body.size();
                    size_t bytes_available = m_buffer.size() - m_idx;
                    size_t copy_size = std::min(bytes_needed, bytes_available);

                    if (copy_size > 0)
                    {
                        m_body.append(&m_buffer[m_idx], copy_size);

                        // @NOTE: must subtract 1 because the m_idx++ in the for loop will add it back.
                        m_idx += (copy_size - 1);
                    }

                    if (m_body.size() == m_content_length)
                    {
                        m_state = S::Done;
                        m_status_code = StatusCode::Ok;
                    }
                }
                else
                {
                    m_state = S::Error;
                    if (m_content_length == 0)
                        m_status_code = StatusCode::LengthRequired;
                    else // @NOTE: should never reach here
                        m_status_code = StatusCode::BadRequest;
                    Logger::error(
                        "RequestParser: body (%ld) is larger than expected size (%ld)",
                        m_body.size(),
                        m_content_length);
                }
                break;
                // Body chunked
            case S::BodyChunkSize:
                Logger::verbose("RequestParser: state - Body Chunk Size: '%c'", m_ch);

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
                    Logger::error("RequestParser: expected a hexadecimal char, but got this '%c' instead", m_ch);
                }
                break;
            case S::BodyChunkSizeExtension:
                Logger::verbose("RequestParser: state - Body Chunk Size Extension: '%c'", m_ch);

                if (m_ch == '\r')
                {
                    m_state = S::BodyChunkSizeCR;
                }
                else
                {
                    // ignore chunk extensions
                }
                break;
            case S::BodyChunkSizeCR:
                Logger::verbose("RequestParser: state - Body Chunk Size CR: '%c'", m_ch);

                if (m_ch != '\n')
                {
                    m_state = S::Error;
                    m_status_code = StatusCode::BadRequest;
                    Logger::error("RequestParser: expected \\n at end of body chunk size");
                }
                else
                {
                    long long hexa = utils::str_tohexadecimal(m_chunk_size_str);
                    if (hexa < 0)
                    {
                        m_state = S::Error;
                        m_status_code = StatusCode::BadRequest;
                        Logger::error("RequestParser: hexadecimal is negative: %ld", hexa);
                        break;
                    }
                    if (hexa > static_cast<long long>(constants::max_body_size))
                    {
                        m_state = S::Error;
                        m_status_code = StatusCode::ContentTooLarge;
                        Logger::error(
                            "RequestParser: chunk size (%ld) larger than max_body_size (%ld)",
                            hexa,
                            constants::max_body_size);
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
                Logger::verbose("RequestParser: state - Body Chunk Data: '%c'", m_ch);

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
                        Logger::error(
                            "RequestParser: body chunk data (%ld) larger than max_body_size (%ld)",
                            m_body.size(),
                            constants::max_body_size);
                    }
                }
                else
                {
                    m_state = S::Error;
                    m_status_code = StatusCode::ContentTooLarge;
                    Logger::error("RequestParser: body chunk larger than expected");
                }
                break;
            case S::BodyChunkDataCR:
                Logger::verbose("RequestParser: state - Body Chunk Data CR: '%c'", m_ch);

                if (m_ch != '\n')
                {
                    m_state = S::Error;
                    m_status_code = StatusCode::BadRequest;
                    Logger::error("RequestParser: expected \\n at end of body chunk data section");
                }
                else
                {
                    m_state = S::BodyChunkSize;
                    m_chunk_size = 0;
                }
                break;
            case S::BodyChunkTrailerKey:
                Logger::verbose("RequestParser: state - Body Chunk Trailer Key: '%c'", m_ch);

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
                    Logger::error("RequestParser: invalid trailer header key char: '%c'", m_ch);
                }
                else
                {
                    m_header_key += m_ch;
                }
                break;
            case S::BodyChunkTrailerValue:
                Logger::verbose("RequestParser: state - Body Chunk Trailer Value: '%c'", m_ch);

                if (m_ch == '\r')
                {
                    // key is normalized
                    std::string k = utils::str_tolower(m_header_key);
                    utils::str_trim_sides(k, constants::body_whitespaces);
                    if (utils::contains(m_headers, k))
                    {
                        m_state = S::Error;
                        m_status_code = StatusCode::BadRequest;
                        Logger::error("RequestParser: duplicated trailer header: %s", k.c_str());
                        break;
                    }

                    utils::str_trim_sides(m_header_value, constants::body_whitespaces);
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
                    Logger::error("RequestParser: invalid trailer value char: '%c'", m_ch);
                }
                break;
            case S::BodyChunkTrailerCR:
                Logger::verbose("RequestParser: state - Body Chunk Trailer CR: '%c'", m_ch);

                if (m_ch != '\n')
                {
                    m_state = S::Error;
                    m_status_code = StatusCode::BadRequest;
                    Logger::error("RequestParser: expected \\n at end of body chunk trailer");
                }
                else
                {
                    m_state = S::BodyChunkTrailerKey;
                }
                break;
            case S::BodyChunkTrailerEndCR:
                Logger::verbose("RequestParser: state - Body Chunk Trailer End CR: '%c'", m_ch);

                if (m_ch != '\n')
                {
                    m_state = S::Error;
                    m_status_code = StatusCode::BadRequest;
                    Logger::error("RequestParser: expected \\n at end of body chunk trailer section");
                }
                else
                {
                    m_state = S::Done;
                    m_status_code = StatusCode::Ok;
                }
                break;
            default:
            {
                m_state = S::Error;
                m_status_code = StatusCode::BadRequest;
                Logger::error("RequestParser: error at default fallthrough");
            }
        }
    }

    if (m_state == S::Done)
    // post parse work
    {
        Logger::info("RequestParser: done");
        // multipart body
        if (utils::contains(m_headers, "content-type"))
        {
            std::string type = m_headers["content-type"];
            if (type.find("multipart/form-data") != std::string::npos)
                m_multipart_body = parse_multipart(type, m_body);
        }
    }
    else
    {
        Logger::debug("RequestParser: parsed %lld bytes so far", m_idx);
    }
}

// https://www.rfc-editor.org/rfc/rfc9110.html#name-field-names
// see 5.6.2
bool RequestParser::is_tchar(char c)
{
    // clang-format off
    return (
        std::isalnum(static_cast<unsigned char>(c)) || 
	c == '!' || c == '#' || c == '$' || 
	c == '%' || c == '&' || c == '\'' || 
	c == '*' || c == '+' || c == '-' || 
	c == '.' || c == '^' || c == '_' || 
	c == '`' || c == '|' || c == '~'
	);
    // clang-format on
}

bool RequestParser::is_vchar(char c)
{
    unsigned char uc = static_cast<unsigned char>(c);
    return uc >= '!' && uc <= '~';
}

bool RequestParser::is_ows(char c)
{
    return c == ' ' || c == '\t';
}

bool RequestParser::is_http_version(const std::string& v)
{
    // clang-format off
    return (
	v == "HTTP/0.9" || 
	v == "HTTP/1.0" || 
	v == "HTTP/1.1" || 
	v == "HTTP/2" || 
	v == "HTTP/3"
	);
    // clang-format on
}

bool RequestParser::is_valid_method(const std::string& m)
{
    // clang-format off
    return (
	 m == "GET" || 
	 m == "PUT" || 
	 m == "POST" || 
	 m == "DELETE" || 
	 m == "CONNECT" || 
	 m == "OPTIONS" || 
	 m == "PATCH" || 
	 m == "TRACE" || 
	 m == "HEAD"
	);
    // clang-format on
}

std::vector<MultiPartBody> RequestParser::parse_multipart(
    const std::string& content_type_header,
    const std::string& body)
{
    Logger::debug("RequestParser: parsing multipart data");

    std::vector<MultiPartBody> parts;

    // extract boundary
    std::string boundary;
    size_t pos = content_type_header.find("boundary=");
    if (pos == std::string::npos)
        return parts; //@NOTE if we don't find a boundary, we treat the multipart body as a raw body
    boundary = "--" + content_type_header.substr(pos + 9);

    // strip whitespaces/quotes
    boundary = boundary.substr(0, boundary.find_first_of(" \t\r\n\""));

    // find first boundary
    pos = body.find(boundary);
    if (pos == std::string::npos)
        return parts;
    pos += boundary.size() + 2; // skip past boundary + \r\n

    // iterate over parts
    while (pos < body.size())
    {
        // check for final boundary "--boundary--"
        if (body.substr(pos, 2) == "--")
            break;

        // split headers and body on first \r\n\r\n
        size_t header_end = body.find("\r\n\r\n", pos);
        if (header_end == std::string::npos)
            break;

        std::string raw_headers = body.substr(pos, header_end - pos);
        pos = header_end + 4; // skip \r\n\r\n

        // find next boundary to know where body ends
        size_t next_boundary = body.find("\r\n" + boundary, pos);
        if (next_boundary == std::string::npos)
            break;

        std::string part_body = body.substr(pos, next_boundary - pos);
        pos = next_boundary + 2 + boundary.size(); // skip \r\n + boundary

        // skip \r\n after boundary (or -- for final boundary)
        if (pos + 2 <= body.size())
            pos += 2;

        // parse part headers
        MultiPartBody part;
        part.body = part_body;
        parse_part_headers(raw_headers, part);
        parts.push_back(part);
    }

    return parts;
}

void RequestParser::parse_part_headers(const std::string& raw_headers, MultiPartBody& part)
{
    std::istringstream stream(raw_headers);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if (line.find("Content-Disposition:") == 0)
        {
            // extract subfields
            // name
            size_t pos = line.find("name=\"");
            if (pos != std::string::npos)
            {
                pos += 6;
                part.name = line.substr(pos, line.find("\"", pos) - pos);
            }
            // filename
            pos = line.find("filename=\"");
            if (pos != std::string::npos)
            {
                pos += 10;
                part.filename = line.substr(pos, line.find("\"", pos) - pos);
            }
        }
        else if (line.find("Content-Type:") == 0)
        {
            part.content_type = line.substr(14);
        }
    }
}

bool MultiPartBody::operator==(const MultiPartBody& other) const
{
    // clang-format off
        return (
		body == other.body &&
		content_type == other.content_type &&
		name == other.name &&
		filename == other.filename
	       );
    // clang-format on
}
