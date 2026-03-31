#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <ostream>
#include <string>

#include "config/types/ServiceConfig.hpp"

// https://github.com/h5bp/server-configs-nginx

class Config
{
  public:
    Config();

    std::vector<ServiceConfig> services;

    void load(const char* file_path);

  private:
    std::string m_file_content;
};

std::ostream& operator<<(std::ostream& os, const Config& cfg);

#endif // CONFIG_HPP
