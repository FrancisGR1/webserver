#include <string>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>      
#include <iostream>

#include "constants.hpp"
#include "MimeTypes.hpp"
#include "Path.hpp"

// Path
Path::Path(const std::string& str_path)
	: exists(false)
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
	init(str_path);
}

Path::Path(const char* cstr_path)
	: exists(false)
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
	init(cstr_path);
}

void Path::init(const std::string& str_path)
{
	if (str_path.empty())
		return;

	raw = str_path;
	if (!raw.empty() && raw.at(raw.size() - 1) == '/')
		ends_with_slash = true;

	// check cgi extension
	size_t dot = raw.find_last_of('.');
	if (dot != std::string::npos)
	{
		size_t ext_end = dot + 3;
		// script path
		cgi_path = raw.substr(0, ext_end);
		std::string extension = cgi_path.substr(dot + 1, cgi_path.size());
		if (extension == constants::py_ext)
		{
			cgi_extension = extension;
			is_cgi = true;
			mime = MimeTypes::from_path(cgi_path);
			// script info
			if (ext_end < raw.length())
				cgi_info = raw.substr(ext_end);
			// script name
			size_t last_slash = cgi_path.rfind('/');
			if (last_slash != std::string::npos)
				cgi_name = cgi_path.substr(last_slash + 1);
			else
				cgi_name = cgi_path;
			raw = cgi_path;
		}
	}

	struct stat st;
	if (stat(raw.c_str(), &st) != 0)
	{
		stat_errno = errno;
		return;
	}
	else
	{
		exists = true;
	}


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
	size  = st.st_size;
	mtime = st.st_mtime;
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
		<< "\tCan read:        " << path.can_read << "\n"
		<< "\tCan write:       " << path.can_write << "\n"
		<< "\tCan exec:        " << path.can_execute << "\n"
		<< "\tSize:            " << path.size << "\n"
		<< "\tTime:            " << path.mtime << "\n";
	return os;
}
