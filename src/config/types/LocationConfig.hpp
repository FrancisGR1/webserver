#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <map>
#include <set>
#include <string>

#include "config/types/Directive.hpp"
#include "config/types/Route.hpp"

//@TODO mudar variáveis para vars privadas
class LocationConfig
{
  public:
    LocationConfig();
    LocationConfig(const std::string name, const std::string root_dir);
    LocationConfig(const std::string name, Directive directive);
    LocationConfig(const std::string name, std::vector<Directive> directives);

    // exclusive to location block scope
    std::string name;
    std::set<std::string> methods;
    //@TODO: mudar para Path
    std::string root_dir;
    std::string upload_dir;
    bool enable_dir_listing;
    bool enable_upload_files;
    std::string default_file;
    std::map<std::string, std::string> cgis;

    // can be set in LocationConfig or ServiceConfig scope
    std::map<size_t, std::string> error_pages;
    Route redirection;
    size_t max_body_size;

    void set(Directive& directive);

  private:
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& lc);

#endif // LOCATIONCONFIG_HPP
