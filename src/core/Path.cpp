#include <string>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>      

#include "constants.hpp"
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
	size_t dot = resolved_path.find_last_of('.');
	if (dot != std::string::npos)
	{
		std::string ext = resolved_path.substr(dot + 1);
		if (ext == constants::py_ext)
			is_cgi = true;
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
