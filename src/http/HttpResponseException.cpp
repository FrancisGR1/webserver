#include "config/ConfigTypes.hpp"
#include "StatusCode.hpp"
#include "HttpResponseException.hpp"

HttpResponseException::HttpResponseException(StatusCode::Code code, const std::string& msg, const LocationConfig& lc)
	: m_status(code)
	, m_location(lc)
	, m_msg(msg) {}

HttpResponseException::HttpResponseException(StatusCode::Code code, const std::string& msg)
	: m_status(code)
	, m_location()
	, m_msg(msg) {}

StatusCode::Code HttpResponseException::status() const        { return m_status; }
const LocationConfig& HttpResponseException::location() const { return m_location; }
const std::string& HttpResponseException::msg() const         { return m_msg; }

HttpResponseException::~HttpResponseException() throw() {}
