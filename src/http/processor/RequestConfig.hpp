#ifndef REQUESTCONFIG_HPP
#define REQUESTCONFIG_HPP

#include <string>

#include "config/types/LocationConfig.hpp"
#include "config/types/ServiceConfig.hpp"
#include "core/Path.hpp"

class RequestConfig
{
  public:
    // construct
    RequestConfig(const ServiceConfig& service);
    RequestConfig(const ServiceConfig& service, const Path& path, const LocationConfig& location);

    // setters
    void set(const Path& path);
    void set(const LocationConfig& location);

    // getters
    const ServiceConfig& service() const;
    const Path& path() const;
    bool has_location() const;
    const LocationConfig* location() const;
    const Route& redirection() const;
    Path index() const;
    Path root() const;
    Path upload_dir() const;
    Path get_error_page_or_nonexistent_path(StatusCode::Code c) const;
    Path cgi_interpreter() const;
    size_t max_body_size() const;

    // checkers
    bool is_cgi() const;
    bool is_redirected() const;
    bool allows_method(const std::string& method) const;
    bool allows_autoindex() const;
    bool has_index() const;
    bool allows_upload() const;
    bool has_upload_dir() const;
    bool has_file_for_error(size_t code) const;

  private:
    const ServiceConfig& m_service;
    Path m_resolved_path; // transformed request path
    const LocationConfig* m_location;
};

std::ostream& operator<<(std::ostream& os, const RequestConfig& config);

#endif // REQUESTCONFIG_HPP
