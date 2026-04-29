#include <utility>

#include "core/Logger.hpp"
#include "core/ResourceLocker.hpp"
#include "core/utils.hpp"

std::map<ResourceLocker::ResourceKey, Path> ResourceLocker::m_resources;

bool ResourceLocker::lock(const Path& path)
{
    return true;
    ResourceLocker::ResourceKey key = make_key(path);

    Logger::debug("ResourceLocker: [LOCK]='%s'", path.c_str());

    if (is_unlocked(path))
    {
        m_resources[key] = path;
        return true;
    }
    else
    {
        Logger::error("ResourceLocker: '%s' is already locked!", path.c_str());
        return false;
    }
}

bool ResourceLocker::unlock(const Path& path)
{
    return true;
    Logger::debug("ResourceLocker: [UNLOCK]='%s'", path.c_str());

    return m_resources.erase(make_key(path)) > 0;
}

bool ResourceLocker::is_unlocked(const Path& path)
{
    return true;
    if (!path.exists)
        Logger::warn("ResourceLocker: path='%s' doesn't exist", path.c_str());

    if (!utils::contains(m_resources, make_key(path)))
    {
        Logger::debug("ResourceLocker: path='%s' is unlocked", path.c_str());
        return true;
    }

    Logger::info("ResourceLocker: path='%s' is locked", path.c_str());
    return false;
}

ResourceLocker::ResourceKey ResourceLocker::make_key(const Path& path)
{
    return std::make_pair(path.dev, path.ino);
}
