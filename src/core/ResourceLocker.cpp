#include <utility>

#include "core/Logger.hpp"
#include "core/ResourceLocker.hpp"
#include "core/contracts.hpp"
#include "core/utils.hpp"

std::map<ResourceLocker::ResourceKey, Path> ResourceLocker::m_resources;

bool ResourceLocker::lock(const Path& path)
{
    ResourceLocker::ResourceKey key = make_key(path);

    Logger::debug("ResourceLocker: lock: '%s'", path.c_str());

    if (!utils::contains(m_resources, key))
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
    Logger::debug("ResourceLocker: unlock: '%s'", path.c_str());

    return m_resources.erase(make_key(path)) > 0;
}

ResourceLocker::ResourceKey ResourceLocker::make_key(const Path& path)
{
    if (!path.exists)
        Logger::warn("ResourceLocker: path doesn't exist");

    return std::make_pair(path.dev, path.ino);
}
