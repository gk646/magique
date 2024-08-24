#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

#include "internal/globals/HandleData.h"

namespace magique
{
    void RegisterDirectHandle(const handle handle, const int id)
    {
        auto& data = global::HANDLE_REGISTRY;
        if (id > data.directHandles.size())
            data.directHandles.resize(id + 1, handle::null);
        data.directHandles[id] = handle;
    }

    void RegisterHandle(const handle handle, const HandleID id)
    {
        auto& registry = global::HANDLE_REGISTRY;
        const int hID = static_cast<int>(id);
        if (registry.handles.size() > hID) [[likely]]
        {
            registry.handles[hID] = handle;
            return;
        }

        if (hID > 100'000)
        {
            LOG_WARNING("Registering with large type number! Likely and error! Skipping");
            return;
        }

        registry.handles.resize(hID + 5);
        registry.handles[hID] = handle;
    }

    void RegisterHandle(handle handle, const char* name)
    {
        auto hash = internal::HashString(name, HASH_SALT);
        MAGIQUE_ASSERT(
            !global::HANDLE_REGISTRY.handleMap.contains(hash),
            "Collision! You either registered a handle twice (with the same name) or suffered an unlucky hash "
            "collision. Change the HASH_SALT parameter until no collisions occur!");
        global::HANDLE_REGISTRY.handleMap.insert({hash, handle});
    }

    handle GetHandle(HandleID type)
    {
        MAGIQUE_ASSERT(static_cast<int>(type) < global::HANDLE_REGISTRY.handles.size(), "No such handle!");
        return global::HANDLE_REGISTRY.handles[static_cast<int>(type)];
    }

    handle GetHandle(const uint32_t hash)
    {
        MAGIQUE_ASSERT(global::HANDLE_REGISTRY.handleMap.contains(hash), "No handle with that name (hash)!");
        return global::HANDLE_REGISTRY.handleMap[hash];
    }

    handle GetDirectHandle(const int id)
    {
        MAGIQUE_ASSERT(global::HANDLE_REGISTRY.directHandles.size() > id, "Handle is not stored!");
        return global::HANDLE_REGISTRY.directHandles[id];
    }

} // namespace magique