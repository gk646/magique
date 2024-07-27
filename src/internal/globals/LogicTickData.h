#ifndef LOGICTICKDATA_H
#define LOGICTICKDATA_H

#include <entt/entity/entity.hpp>
#include <cxstructs/BitMask.h>

#include <magique/fwd.hpp>
#include <magique/internal/DataStructures.h>
#include <magique/util/Defines.h>

#include "internal/datastructures/MultiResolutionGrid.h"

namespace magique
{
    struct PairInfo final // Saves entity id and type
    {
        entt::entity e1;
        EntityID id1;
        entt::entity e2;
        EntityID id2;
    };

    template <typename T>
    struct AlignedVec final
    {
        // To prevent false sharing
        alignas(64) vector<T> vec;
    };

    using CollPairCollector = std::array<AlignedVec<PairInfo>, MAGIQUE_WORKER_THREADS + 1>;
    using EntityCollector = std::array<AlignedVec<entt::entity>, MAGIQUE_WORKER_THREADS + 1>;
    using EntityFlagMap = HashMap<entt::entity, cxstructs::EnumMask<UpdateFlag>>;
    using EntityCache = HashMap<entt::entity, uint16_t>;
    using HashGrid = SingleResolutionHashGrid<entt::entity, 32>;

    struct LogicTickData final
    {
        MapID cameraMap;                                     // Map the camera is in
        Camera2D camera{};                                   // current camera
        entt::entity cameraEntity = entt::null;              // entity id of the camera
        std::array<MapID, MAGIQUE_MAX_PLAYERS> loadedMaps{}; // Currently loaded zones
        EntityFlagMap changedSet{500};                       // Change set for multiplayer events
        EntityCache entityUpdateCache{1000};                 // Caches entites not in update range anymore
        vector<entt::entity> entityUpdateVec;                // vector containing the entites to update for this tick
        vector<entt::entity> collisionVec{};                 // vector containing the entites to check for collision
        vector<entt::entity> drawVec;                        // vector containing all entites to be drawn this tick
        CollPairCollector collisionPairs{};                  // Collision pair collectors
        HashSet<uint64_t> pairSet{};                         // Filters unique collision pairs
        EntityCollector collectors{};                        // Collects entities - 2 for the 2 worker threads
        HashGrid hashGrid{200};                              // Global hashGrid for all entities
        std::atomic_flag flag; // Atomic spinlock - whenever any data is accessed on the draw thread

        LogicTickData()
        {
            hashGrid.reserve(150, 1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
            collisionVec.reserve(500);
            pairSet.reserve(1000);
        }

        void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire))
            {
            }
        }

        void unlock() { flag.clear(std::memory_order_release); }

        void clear()
        {
            changedSet.clear();
            entityUpdateCache.clear();
            entityUpdateVec.clear();
            drawVec.clear();
            hashGrid.clear();
        }
    };

    namespace global
    {
        inline LogicTickData LOGIC_TICK_DATA;
    }

} // namespace magique

#endif //LOGICTICKDATA_H