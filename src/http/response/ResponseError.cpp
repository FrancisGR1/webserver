#include <stdexcept>

#include "config/ConfigTypes.hpp"
#include "config/types/LocationConfig.hpp"
#include "http/StatusCode.hpp"
#include "ResponseError.hpp"

ResponseError::ResponseError(StatusCode::Code code, const std::string& msg, const RequestContext* ctx)
	: m_status(code)
	, m_msg(msg)
	, m_ctx(ctx) {}

StatusCode::Code ResponseError::status() const        { return m_status; }
const std::string& ResponseError::msg() const         { return m_msg; }
bool ResponseError::has_ctx() const         	      { return m_ctx != NULL; }
const RequestContext& ResponseError::ctx() const         
{ 
	if (m_ctx == NULL)
        	throw std::runtime_error("ResponseError: Tried to access Null Pointer!");
	return *m_ctx; 
}

ResponseError::~ResponseError() throw() {}
