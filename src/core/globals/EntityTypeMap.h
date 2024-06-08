#ifndef ENTITYTYPEMAP_H
#define ENTITYTYPEMAP_H

#include <magique/util/DataStructures.h>
#include <entt/fwd.hpp>

namespace magique::global{


    inline HashMap<EntityID, std::function<void(entt::registry&, entt::entity)>> ENT_TYPE_MAP{50};
}



#endif //ENTITYTYPEMAP_H