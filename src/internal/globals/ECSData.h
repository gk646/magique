#ifndef MAGIQUE_ECSDATA_H
#define MAGIQUE_ECSDATA_H

#include "internal/datastructures/HashTypes.h"

namespace magique
{
    struct ECSData final
    {
        uint32_t entityID = 0;
        HashMap<EntityID, CreateFunc> typeMap{50};
    };

    namespace global
    {
        inline ECSData ECS_DATA{};
    }

} // namespace magique


#endif //MAGIQUE_ECSDATA_H