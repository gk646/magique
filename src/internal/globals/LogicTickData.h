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
    template <typename T>
    struct AlignedVec
    {
        // To prevent false sharing
        alignas(64) vector<T> vec;
    };

    using CollPairCollector = std::array<AlignedVec<std::pair<entt::entity, entt::entity>>, MAGIQUE_WORKER_THREADS + 1>;
    using EntityCollector = std::array<AlignedVec<entt::entity>, MAGIQUE_WORKER_THREADS + 1>;

    struct LogicTickData final
    {
        // Map the camera is in
        MapID cameraMap;

        // current camera
        Camera2D camera{};

        // entity id of the camera
        entt::entity cameraEntity = entt::null;

        // Currently loaded zones
        std::array<MapID, MAGIQUE_MAX_PLAYERS> loadedMaps{};

        // Change set for multiplayer events
        HashMap<entt::entity, cxstructs::EnumMask<UpdateFlag>> changedSet{500};

        // Cache for entities that are not in update range anymore
        // They are still updated for cache duration
        HashMap<entt::entity, uint16_t> entityUpdateCache{1000};

        // vector containing the entites to update for this tick
        vector<entt::entity> entityUpdateVec;

        // vector containing the entites to check for collision
        vector<entt::entity> collisionVec{};

        // vector containing all entites to be drawn this tick
        // Culled with the camera
        vector<entt::entity> drawVec;

        // Collision pair collectors
        CollPairCollector collisionPairs{};

        // Collects entities - 2 for the 2 worker threads
        EntityCollector collectors{};

        // Global hashGrid for all entities
        SingleResolutionHashGrid<entt::entity, 32> hashGrid{200};

        // Shadow segments
        vector<Vector3> shadowQuads;

        // Atomic spinlock - whenever any data is accessed on the draw thread
        std::atomic_flag flag;

        LogicTickData()
        {
            hashGrid.reserve(150, 1000);
            shadowQuads.reserve(1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
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