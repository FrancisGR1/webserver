#include <string>

class AMethodHandler
{
	public:
		virtual const std::string& status_line() const = 0;
		virtual const std::string& headers() const = 0;
		virtual const std::string& body() const = 0;
		virtual const Path& body_file_path() const = 0;
		virtual int body_fd() const = 0;
		virtual int body_type() const = 0;
};
