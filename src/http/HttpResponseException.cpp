#include "config/ConfigTypes.hpp"
#include "config/types/LocationConfig.hpp"
#include "StatusCode.hpp"
#include "HttpResponseException.hpp"

HttpResponseException::HttpResponseException(StatusCode::Code code, const std::string& msg, const LocationConfig& lc)
	: m_status(code)
	, m_msg(msg) {}

StatusCode::Code HttpResponseException::status() const        { return m_status; }
const std::string& HttpResponseException::msg() const         { return m_msg; }

HttpResponseException::~HttpResponseException() throw() {}
