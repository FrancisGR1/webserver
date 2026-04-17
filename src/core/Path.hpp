#ifndef PATH_HPP
#define PATH_HPP

#include <string>
#include <sys/types.h>

//@TODO: tornar class
// adicionar fd
struct Path
{
    Path(const std::string& str_path);
    Path(const char* cstr_path);
    void init(const std::string& str_path);

    //@TODO:
    // string raw
    // string str
    // resolve(Location)?

    bool exists;
    bool ends_with_slash;

    // type
    std::string mime;
    bool is_directory;
    bool is_regular_file;
    bool is_cgi;
    bool is_absolute;
    bool is_relative;

    // cgi
    std::string cgi_path;
    std::string cgi_info;
    std::string cgi_name;
    std::string cgi_extension;

    // permissions
    bool can_read;
    bool can_write;
    bool can_execute;

    // meta-data
    off_t size;
    time_t mtime;

    //@QUESTION @TODO: eliminar?
    int stat_errno;

    std::string raw;

    //@TODO: funções
    // write
    // resolve
};

std::ostream& operator<<(std::ostream& os, const Path& path);

#endif // PATH_HPP
