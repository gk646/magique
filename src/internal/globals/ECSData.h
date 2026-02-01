// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ECSDATA_H
#define MAGIQUE_ECSDATA_H

#include <magique/util/Datastructures.h>

namespace magique
{
    struct ECSData final
    {
        uint32_t entityID = 1;
        HashMap<EntityType, CreateFunc> typeMap{50};
    };

    namespace global
    {
        inline ECSData ECS_DATA{};
    }

} // namespace magique


#endif //MAGIQUE_ECSDATA_H