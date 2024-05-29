#ifndef COREDATA_H
#define COREDATA_H

#include <entt/entity/registry.hpp>

#include <magique/util/Logging.h>
#include <magique/ecs/Registry.h>
#include <magique/core/DataStructures.h>


#include "InternalTypes.h"
#include "datastructures/HashGrid.h"

// All the core data that is shared between internal implementations
// We carry a slog of includes around but dont really care as its statically compiled
// Library compiled time isnt a user cost


namespace magique
{
    inline util::LogLevel LOG_LEVEL = util::LEVEL_NONE; // All logs visible

    // Internal registry
    inline entt::registry ENTT_REGISTRY;

    // Type map
    inline HashMap<EntityType, std::function<void(entt::registry&, entt::entity)>> ENT_TYPE_MAP;

    // Current logic data
    inline LogicTickData LOGIC_TICK_DATA;

    // Current draw data
    inline DrawTickData DRAW_TICK_DATA;

} // namespace magique
#endif //COREDATA_H