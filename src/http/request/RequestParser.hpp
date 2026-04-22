#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <map>
#include <string>
#include <vector>

#include "http/StatusCode.hpp"

class Request;

struct ParserState
{
    enum Enum
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

struct MultiPartBody;

class RequestParser
{
  public:
    RequestParser();

    void feed(const char* raw);
    void feed(const std::string& raw);
    void feed(char c);
    bool done() const;
    ParserState::Enum state() const;
    Request get() const;
    bool error() const;
    void clear();

  private:
    // http elements
    // status
    std::string m_method;
    std::string m_target_path;
    std::string m_target_query;
    std::string m_protocol_version;
    StatusCode::Code m_status_code;
    // headers
    std::map<std::string, std::string> m_headers;
    // body
    std::string m_body;
    std::vector<MultiPartBody> m_multipart_body;

    // parsing state
    ParserState::Enum m_state;
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

    // main logic
    void parse();

    // header specific char validation
    bool is_tchar(char c);
    bool is_vchar(char c);
    bool is_ows(char c);

    // utils
    bool is_http_version(const std::string& v);
    bool is_valid_method(const std::string& m);
    std::vector<MultiPartBody> parse_multipart(const std::string& content_type_header, const std::string& body);
    void parse_part_headers(const std::string& raw_headers, MultiPartBody& part);
};

struct MultiPartBody
{
    std::string body;
    std::string content_type;
    std::string name;
    std::string filename;

    bool operator==(const MultiPartBody& other) const;
};

#endif // REQUESTPARSER_HPP
