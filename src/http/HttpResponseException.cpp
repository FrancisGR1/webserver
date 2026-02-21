#include "config/ConfigTypes.hpp"
#include "config/types/LocationConfig.hpp"
#include "StatusCode.hpp"
#include "HttpResponseException.hpp"

ResponseError::ResponseError(StatusCode::Code code, const std::string& msg)
	: m_status(code)
	, m_msg(msg) {}

StatusCode::Code ResponseError::status() const        { return m_status; }
const std::string& ResponseError::msg() const         { return m_msg; }

ResponseError::~ResponseError() throw() {}
