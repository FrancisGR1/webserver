#include <string>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>      

#include "MimeTypes.hpp"
#include "Path.hpp"

// Path
Path::Path(const std::string& resolved_path)
	: exists(false)
	, mime(MimeTypes::from_path(resolved_path))
	, ends_with_slash(false)
	, is_directory(false)
	, is_regular_file(false)
	, is_cgi(false)
	, can_read(false)
	, can_write(false)
	, can_execute(false)
	, size(0)
	, mtime(0)
	, stat_errno(0)
	, resolved(resolved_path)
{
	init(resolved_path);
}

Path::Path(const char* resolved_path)
	: exists(false)
	, mime(MimeTypes::from_path(resolved_path))
	, ends_with_slash(false)
	, is_directory(false)
	, is_regular_file(false)
	, is_cgi(false)
	, can_read(false)
	, can_write(false)
	, can_execute(false)
	, size(0)
	, mtime(0)
	, stat_errno(0)
	, resolved(resolved_path)
{
	const std::string rp = resolved_path;
	init(rp);
}

void Path::init(const std::string& resolved_path)
{
	if (!resolved_path.empty() &&
			resolved_path.at(resolved_path.size() - 1) == '/')
		ends_with_slash = true;

	struct stat st;
	if (stat(resolved_path.c_str(), &st) != 0)
	{
		stat_errno = errno;
		return;
	}

	exists = true;

	//  file type
	if (S_ISDIR(st.st_mode))
		is_directory = true;
	else if (S_ISREG(st.st_mode))
		is_regular_file = true;

	// check cgi extension
	size_t dot = resolved_path.find_first_of('.');
	if (dot != std::string::npos)
	{
		is_cgi = true;
		size_t py_end = dot + 3;
		// script path
		std::string cgi_path = resolved_path.substr(0, py_end);
		// script info
		if (py_end < resolved_path.length())
			cgi_info = resolved_path.substr(py_end);
		// script name
		size_t last_slash = cgi_path.rfind('/');
		if (last_slash != std::string::npos)
			cgi_name = cgi_path.substr(last_slash + 1);
		else
			cgi_name = cgi_path;
	}

	// check permissions
	if (access(resolved_path.c_str(), R_OK) == 0)
		can_read = true;
	if (access(resolved_path.c_str(), X_OK) == 0)
		can_execute = true;
	if (access(resolved_path.c_str(), W_OK) == 0)
		can_write = true;

	// data
	size  = st.st_size;
	mtime = st.st_mtime;

}
