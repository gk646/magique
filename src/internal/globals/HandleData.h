// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_HANDLEREGISTRY_H
#define MAGIQUE_HANDLEREGISTRY_H

#include "internal/datastructures/HashTypes.h"

namespace magique
{
    struct HandleRegistry final
    {
        HashMap<uint32_t, handle> handleMap{100};
     std::vector<handle> handles;
     std::vector<handle> directHandles;

        HandleRegistry()
        {
            directHandles.resize(50, handle::null);
            handles.resize(100, handle::null);
        }
    };

    namespace global
    {
        inline HandleRegistry HANDLE_REGISTRY{};

    }
} // namespace magique


#endif //MAGIQUE_HANDLEREGISTRY_H