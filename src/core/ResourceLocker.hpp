#ifndef RESOURCE_LOCKER_HPP
#define RESOURCE_LOCKER_HPP

#include <map>
#include <utility>

#include <dirent.h>

#include "core/Path.hpp"

class ResourceLocker
{
  public:
    static bool lock(const Path& path);
    static bool unlock(const Path& path);
    static bool is_unlocked(const Path& path);

  private:
    typedef std::pair<dev_t, ino_t> ResourceKey;
    static std::map<ResourceKey, Path> m_resources;

    // utils
    static ResourceKey make_key(const Path& path);
};

#endif // RESOURCE_LOCKER
