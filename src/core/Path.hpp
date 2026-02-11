#ifndef PATH_HPP
#define PATH_HPP

#include <string>
#include <sys/types.h>

struct Path
{
	Path(const std::string& resolved_path);

	//@TODO:
	//string raw
	//string resolved
	//resolve(Location)?

	bool exists;
	std::string mime;
	bool ends_with_slash;

	bool is_directory;
	bool is_regular_file;
	bool is_cgi;
	//@TODO:
	//is absolute
	//is relative

	// cgi
	std::string cgi_path;
	std::string cgi_info;
	std::string cgi_name;

	bool can_read;
	bool can_write;
	bool can_execute;

	off_t size;
	time_t mtime;

	//@QUESTION @TODO: eliminar?
	int stat_errno;

	std::string resolved;

	//@TODO: funções
	//write
	//resolve
};
#endif // PATH_HPP
