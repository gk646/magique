#ifndef HANDLEREGISTRY_H
#define HANDLEREGISTRY_H

#include <vector>

#include <magique/internal/DataStructures.h>
#include <magique/util/Defines.h>

namespace magique{
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

namespace global{
    inline HandleRegistry HANDLE_REGISTRY{};

    }
    }


#endif //HANDLEREGISTRY_H