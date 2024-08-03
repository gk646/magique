#ifndef HANDLEREGISTRY_H
#define HANDLEREGISTRY_H

#include <magique/util/Defines.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/HashTypes.h"

namespace magique
{
    struct HandleRegistry final
    {
        vector<handle> handles;
        HashMap<uint32_t, handle> handleMap{100};
        uint16_t fastHandles[MAGIQUE_DIRECT_HANDLES]{};

        HandleRegistry()
        {
            // Intitialize to null handles for 16 bit
            memset(fastHandles, UINT16_MAX, MAGIQUE_DIRECT_HANDLES * sizeof(uint16_t));
            handles.resize(100);
        }
    };

    namespace global
    {
        inline HandleRegistry HANDLE_REGISTRY{};

    }
} // namespace magique


#endif //HANDLEREGISTRY_H