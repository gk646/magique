#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

#include "core/globals/HandleRegistry.h"

namespace magique
{
    void RegisterDirectHandle(handle handle, const int id)
    {
        static_assert(MAGIQUE_DIRECT_HANDLES < UINT16_MAX && "Has to fit into 16 bit");
        M_ASSERT(id < MAGIQUE_DIRECT_HANDLES, "Out of bounds! Use util/Defines.h to adjust the size");
        global::HANDLE_REGISTRY.fastHandles[id] = static_cast<uint16_t>(handle);
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

        LOG_INFO("Resizing the handle vector to size: %d", hID + 1);
        registry.handles.resize(hID + 1);
        registry.handles[hID] = handle;
    }

    void RegisterHandle(handle handle, const char* name)
    {
        auto hash = HashString(name, HASH_SALT);
        M_ASSERT(!global::HANDLE_REGISTRY.handleMap.contains(hash),
                 "Collision! You either registered a handle twice (with the same name) or suffered an unlucky hash "
                 "collision. Change the HASH_SALT parameter until no collisions occur!");
        global::HANDLE_REGISTRY.handleMap.insert({hash, handle});
    }

    handle GetHandle(HandleID type)
    {
        M_ASSERT(static_cast<int>(type) < global::HANDLE_REGISTRY.handles.size(), "No such handle!");
        return global::HANDLE_REGISTRY.handles[static_cast<int>(type)];
    }

    handle GetHandle(const uint32_t hash)
    {
        M_ASSERT(global::HANDLE_REGISTRY.handleMap.contains(hash), "No handle with that name (hash)!");
        return global::HANDLE_REGISTRY.handleMap[hash];
    }

    handle GetDirectHandle(const int id)
    {
        M_ASSERT(id < MAGIQUE_DIRECT_HANDLES, "Out of bounds! Use util/Defines.h to adjust the size");
        M_ASSERT(global::HANDLE_REGISTRY.fastHandles[id] != UINT16_MAX, "Null handle");
        return static_cast<handle>(global::HANDLE_REGISTRY.fastHandles[id]);
    }

} // namespace magique