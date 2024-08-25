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
        entt::entity e1;
        entt::entity e2;
        EntityType id1;
        EntityType id2;
    };

    template <typename T>
    struct AlignedVec final
    {
        // To prevent false sharing
        alignas(64) vector<T> vec;
    };

    using CollPairCollector = AlignedVec<PairInfo>[MAGIQUE_WORKER_THREADS + 1];
    using EntityCollector = AlignedVec<entt::entity>[MAGIQUE_WORKER_THREADS + 1];
    using EntityHashGrid = SingleResolutionHashGrid<entt::entity, 31>;

    struct DynamicCollisiondata final
    {
        HashSet<uint64_t> pairSet;                            // Filters unique collision pairs
        CollPairCollector collisionPairs{};                   // Collision pair collectors
        EntityHashGrid hashGrid{MAGIQUE_COLLISION_CELL_SIZE}; // Global hashGrid for all entities

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