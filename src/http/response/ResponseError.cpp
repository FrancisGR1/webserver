#include <ostream>
#include <stdexcept>

#include "ResponseError.hpp"
#include "http/StatusCode.hpp"

ResponseError::ResponseError(StatusCode::Code code, const std::string& msg, const RequestContext* ctx)
    : m_status(code)
    , m_msg(msg)
    , m_ctx(ctx)
{
}

StatusCode::Code ResponseError::status_code() const
{
    return m_status;
}

const std::string& ResponseError::msg() const
{
    return m_msg;
}

bool ResponseError::has_ctx() const
{
    return m_ctx != NULL;
}

const RequestContext& ResponseError::ctx() const
{
    if (m_ctx == NULL)
        throw std::logic_error("ResponseError: Tried to access Null Pointer!");
    return *m_ctx;
}

void ResponseError::set(RequestContext* ctx)
{
    if (ctx)
    {
        m_ctx = ctx;
    }
}

ResponseError::~ResponseError() throw()
{
}

std::ostream& operator<<(std::ostream& os, const ResponseError& error)
{
    os << "Error:\n\tStatus: " << error.status_code() << " " << StatusCode::to_reason(error.status_code())
       << "\n\tMsg: " << error.msg() << "\n";
    return os;
}
