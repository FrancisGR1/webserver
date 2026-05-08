#ifndef RESPONSEERROR_HPP
#define RESPONSEERROR_HPP

#include <string>

#include "http/StatusCode.hpp"
#include "http/processor/RequestContext.hpp"

class ResponseError : public std::exception
{
  public:
    // construct/destruct
    ResponseError(StatusCode::Code code, const std::string& msg, const RequestContext* ctx = NULL);
    virtual ~ResponseError() throw();

    // getters
    StatusCode::Code status_code() const;
    const std::string& msg() const;
    bool has_ctx() const;
    const RequestContext& ctx() const;

    // setters
    void set(RequestContext* ctx);

  private:
    StatusCode::Code m_status;
    const std::string m_msg;
    const RequestContext* m_ctx; // not owned, is nullable

    // illegal
    ResponseError();
};

std::ostream& operator<<(std::ostream& os, const ResponseError& error);

#endif // RESPONSEERROR_HPP
