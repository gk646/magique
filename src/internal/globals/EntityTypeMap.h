#ifndef ENTITYTYPEMAP_H
#define ENTITYTYPEMAP_H

#include <magique/internal/DataStructures.h>
#include <entt/entity/fwd.hpp>

namespace magique
{
    struct ECSData final
    {
        uint32_t entityID = 0;
        HashMap<EntityID, std::function<void(entt::registry&, entt::entity)>> typeMap{50};
    };

    namespace global
    {
        inline ECSData ECS_DATA{};

    }

} // namespace magique


#endif //ENTITYTYPEMAP_H