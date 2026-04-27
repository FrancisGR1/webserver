#include <cerrno>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "core/MimeTypes.hpp"
#include "core/Path.hpp"
#include "core/constants.hpp"
#include "core/contracts.hpp"

Path::Path()
    : exists(false)
    , dev(0)
    , ino(0)
    , fd(-1)
    , ends_with_slash(false)
    , is_directory(false)
    , is_regular_file(false)
    , is_cgi(false)
    , is_absolute(false)
    , is_relative(false)
    , can_read(false)
    , can_write(false)
    , can_execute(false)
    , size(0)
    , mtime(0)
    , stat_errno(0)
    , raw("")
{
    Logger::verbose("Path: default constructor");
}

Path::Path(const std::string& str_path)
    : exists(false)
    , dev(0)
    , ino(0)
    , fd(-1)
    , ends_with_slash(false)
    , mime(MimeTypes::from_path(str_path))
    , is_directory(false)
    , is_regular_file(false)
    , is_cgi(false)
    , is_absolute(false)
    , is_relative(false)
    , can_read(false)
    , can_write(false)
    , can_execute(false)
    , size(0)
    , mtime(0)
    , stat_errno(0)
    , raw(str_path)
{
    Logger::verbose("Path: string constructor");
    init(str_path);
}

Path::Path(const char* cstr_path)
    : exists(false)
    , dev(0)
    , ino(0)
    , fd(-1)
    , ends_with_slash(false)
    , mime(MimeTypes::from_path(cstr_path))
    , is_directory(false)
    , is_regular_file(false)
    , is_cgi(false)
    , is_absolute(false)
    , is_relative(false)
    , can_read(false)
    , can_write(false)
    , can_execute(false)
    , size(0)
    , mtime(0)
    , stat_errno(0)
    , raw(cstr_path)
{
    Logger::verbose("Path: const char* constructor");
    init(cstr_path);
}

void Path::set_cgi()
{
    // find dot
    size_t dot = raw.find_last_of('.');
    if (dot == std::string::npos)
        return;

    // find pattern
    size_t ext_end = raw.size();
    std::string ext;
    for (size_t i = 0; i < constants::extensions_num; ++i)
    {
        ext = raw.substr(dot, constants::extensions[i].size() + 1);
        if (ext == constants::extensions[i])
        {
            ext_end = dot + constants::extensions[i].size();
            if (ext_end < raw.size() && (raw[ext_end] != '/' && raw[ext_end] != '?' && raw[ext_end] != '#'))
                continue;
            is_cgi = true;
            break;
        }
    }
    if (!is_cgi)
        return;

    // script
    // path
    cgi_path = raw.substr(0, dot + ext.size());
    cgi_extension = ext;
    mime = MimeTypes::from_path(cgi_path);
    // info
    if (ext_end < raw.length())
        cgi_info = raw.substr(ext_end);
    // name
    size_t last_slash = cgi_path.rfind('/');
    if (last_slash != std::string::npos)
    {
        cgi_name = cgi_path.substr(last_slash + 1);
        cgi_dir = cgi_path.substr(0, last_slash);
    }
    else
    {
        cgi_name = cgi_path;
        cgi_dir = ".";
    }
    // path without extra postfix info (if it has any)
    // ex.: /scripts/script.py?info=data
    raw = cgi_path;

    Logger::trace("Path: cgi: '%s'", raw.c_str());
}

const char* Path::c_str() const
{
    return raw.c_str();
}

void Path::init(const std::string& str_path)
{
    Logger::trace("Path: initialize");

    if (str_path.empty())
        return;

    raw = str_path;
    if (!raw.empty() && raw.at(raw.size() - 1) == '/')
        ends_with_slash = true;

    // stat
    struct stat st;
    if (stat(raw.c_str(), &st) != 0)
    {
        stat_errno = errno;
        return;
    }
    else
    {
        exists = true;
        dev = st.st_dev;
        ino = st.st_ino;
    }

    // cgi
    set_cgi();

    //  file type
    if (S_ISDIR(st.st_mode))
        is_directory = true;
    else if (S_ISREG(st.st_mode))
        is_regular_file = true;

    // check permissions
    if (access(raw.c_str(), R_OK) == 0)
        can_read = true;
    if (access(raw.c_str(), X_OK) == 0)
        can_execute = true;
    if (access(raw.c_str(), W_OK) == 0)
        can_write = true;

    // data
    size = st.st_size;
    mtime = st.st_mtime;
}

int Path::open(int flags, int permissions)
{
    int fd = ::open(c_str(), flags, permissions);
    if (fd > -1) // reinit
        init(c_str());
    return fd;
}

std::ostream& operator<<(std::ostream& os, const Path& path)
{
    os << std::boolalpha;
    os << "Path data\n"
       << "\tPath:            " << path.raw << "\n"
       << "\tExists:          " << path.exists << "\n"
       << "\tMime:            " << path.mime << "\n"
       << "\tEnds with /:     " << path.ends_with_slash << "\n"
       << "\tIs directory:    " << path.is_directory << "\n"
       << "\tIs regular file: " << path.is_regular_file << "\n"
       << "\tIs cgi:          " << path.is_cgi << "\n"
       << "\tCgi Path:        " << path.cgi_path << "\n"
       << "\tCgi Info:        " << path.cgi_info << "\n"
       << "\tCgi Name:        " << path.cgi_name << "\n"
       << "\tCgi Ext:         " << path.cgi_extension << "\n"
       << "\tCan read:        " << path.can_read << "\n"
       << "\tCan write:       " << path.can_write << "\n"
       << "\tCan exec:        " << path.can_execute << "\n"
       << "\tSize:            " << path.size << "\n"
       << "\tTime:            " << path.mtime << "\n";
    return os;
}

bool Path::operator<(const Path& other) const
{
    if (exists && other.exists)
    {
        if (dev != other.dev)
            return dev < other.dev;
        return ino < other.ino;
    }

    INVARIANT(false, "Path should exist if it's going to be compared");
    return false; // unreachable
}
