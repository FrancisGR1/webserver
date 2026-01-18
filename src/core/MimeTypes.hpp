#ifndef MIME_TYPES
#define MIME_TYPES

#include <string>
#include <map>

class MimeTypes
{
	public:
		static std::string from_path(const std::string& path);

	private:
		static const std::map<std::string, std::string>& mimes();
		static std::map<std::string, std::string> make_mimes();
};

#endif // MIME_TYPES

