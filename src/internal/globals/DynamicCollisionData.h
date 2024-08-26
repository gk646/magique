#ifndef DYNAMICCOLLISIONDATA_H
#define DYNAMICCOLLISIONDATA_H

#include "internal/datastructures/HashTypes.h"
#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/MultiResolutionGrid.h"

namespace magique
{
    struct PairInfo final // Saves entity id and type
    {
        CollisionInfo info;
        PositionC& pos1;
        PositionC& pos2;
        entt::entity e1;
        entt::entity e2;
    };

    using CollPairCollector = AlignedVec<PairInfo>[MAGIQUE_WORKER_THREADS + 1];
    using EntityCollector = AlignedVec<entt::entity>[MAGIQUE_WORKER_THREADS + 1];
    using EntityHashGrid = SingleResolutionHashGrid<entt::entity, MAGIQUE_MAX_ENTITIES_CELL, MAGIQUE_COLLISION_CELL_SIZE>;

    struct DynamicCollisiondata final
    {
        HashSet<uint64_t> pairSet;          // Filters unique collision pairs
        EntityHashGrid hashGrid;            // Global hashGrid for all dynamic entities
        CollPairCollector collisionPairs{}; // Collision pair collectors

        DynamicCollisiondata()
        {
            hashGrid.reserve(150, 1000);
            pairSet.reserve(1000);
        }
    };

    namespace global
    {

        inline DynamicCollisiondata DY_COLL_DATA{};
    }
} // namespace magique
#endif //DYNAMICCOLLISIONDATA_H