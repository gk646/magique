#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/util/Macros.h>

#include "core/CoreData.h"


namespace magique
{
    struct HandleRegistry final
    {
        std::vector<handle> handles;
        HashMap<uint32_t, handle> handleMap{100};
        uint16_t fastHandles[MAGIQUE_DIRECT_HANDLES]{};

        HandleRegistry()
        {
            // Intitialize to null handles for 16 bit
            memset(fastHandles, UINT16_MAX, MAGIQUE_DIRECT_HANDLES * sizeof(uint16_t));
            handles.resize(100);
            handleMap.reserve(50);
        }
    };

    inline HandleRegistry HANDLE_REGISTRY{};

    void RegisterDirectHandle(handle handle, const int id)
    {
        static_assert(MAGIQUE_DIRECT_HANDLES < UINT16_MAX && "Has to fit into 16 bit");
        M_ASSERT(id < MAGIQUE_DIRECT_HANDLES, "Out of bounds! Use util/Defines.h to adjust the size");
        HANDLE_REGISTRY.fastHandles[id] = static_cast<uint16_t>(handle);
    }


    void RegisterHandle(const handle handle, const HandleID id)
    {
        const int hID = static_cast<int>(id);
        if (HANDLE_REGISTRY.handles.size() > hID) [[likely]]
        {
            HANDLE_REGISTRY.handles[hID] = handle;
            return;
        }

        if (hID > 100'000)
        {
            LOG_WARNING("Registering with large type number! Likely and error! Skipping");
            return;
        }

        LOG_INFO("Resizing the handle vector to size: %d", hID + 1);
        HANDLE_REGISTRY.handles.resize(hID + 1);
        HANDLE_REGISTRY.handles[hID] = handle;
    }

    void RegisterHandle(handle handle, const char* name)
    {
        auto hash = util::HashString(name, HASH_SALT);
        M_ASSERT(!HANDLE_REGISTRY.handleMap.contains(hash),
                 "Collision! You either registered a handle twice (with the same name) or suffered an unlucky hash "
                 "collision. Change the HASH_SALT parameter until no collisions occur!");
        HANDLE_REGISTRY.handleMap.insert({hash, handle});
    }

    handle GetHandle(HandleID type)
    {
        M_ASSERT((int)type < HANDLE_REGISTRY.handles.size(), "No such handle!");
        return HANDLE_REGISTRY.handles[static_cast<int>(type)];
    }

    handle GetHandle(const uint32_t hash)
    {
        M_ASSERT(HANDLE_REGISTRY.handleMap.contains(hash), "No entry with that name!");
        return HANDLE_REGISTRY.handleMap[hash];
    }

    handle GetDirectHandle(const int id)
    {
        M_ASSERT(id < MAGIQUE_DIRECT_HANDLES, "Out of bounds! Use util/Defines.h to adjust the size");
        M_ASSERT(HANDLE_REGISTRY.fastHandles[id] != UINT16_MAX, "Null handle");
        return static_cast<handle>(HANDLE_REGISTRY.fastHandles[id]);
    }

} // namespace magique