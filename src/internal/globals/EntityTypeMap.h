#ifndef ENTITYTYPEMAP_H
#define ENTITYTYPEMAP_H

#include <magique/internal/DataStructures.h>
#include <entt/fwd.hpp>

namespace magique::global
{

    inline HashMap<EntityID, std::function<void(entt::registry&, entt::entity)>> ENT_TYPE_MAP{50};
}


#endif //ENTITYTYPEMAP_H