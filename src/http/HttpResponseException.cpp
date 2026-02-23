#include <stdexcept>

#include "config/ConfigTypes.hpp"
#include "config/types/LocationConfig.hpp"
#include "StatusCode.hpp"
#include "HttpResponseException.hpp"

ResponseError::ResponseError(StatusCode::Code code, const std::string& msg)
	: m_status(code)
	, m_msg(msg)
	, m_ctx(NULL) {}

ResponseError::ResponseError(StatusCode::Code code, const std::string& msg, HttpRequestContext& ctx)
	: m_status(code)
	, m_msg(msg)
	, m_ctx(&ctx) {}

StatusCode::Code ResponseError::status() const        { return m_status; }
const std::string& ResponseError::msg() const         { return m_msg; }
bool ResponseError::has_ctx() const         	      { return m_ctx != NULL; }
const HttpRequestContext& ResponseError::ctx() const         
{ 
	if (m_ctx == NULL)
        	throw std::runtime_error("ResponseError: Tried to access Null Pointer!");
	return *m_ctx; 
}

ResponseError::~ResponseError() throw() {}
