#ifndef MIME_TYPES
#define MIME_TYPES

#include <map>
#include <string>

class MimeTypes
{
  public:
    static std::string from_path(const std::string& path);
    static std::string from_extension(const std::string& ext);

  private:
    static const std::map<std::string, std::string>& mimes();
    static std::map<std::string, std::string> make_mimes();
};

#endif // MIME_TYPES
