#ifndef LOGICTICKDATA_H
#define LOGICTICKDATA_H

#include <magique/fwd.hpp>
#include <magique/util/DataStructures.h>
#include <cxstructs/BitMask.h>

#include "util/datastructures/MultiResolutionGrid.h"
#include "core/Config.h"

#include <entt/entity/entity.hpp>

namespace magique
{
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

        // vector containing the entites to update for this ticka
        vector<entt::entity> entityUpdateVec;

        // vector containing all entites to be drawn this tick
        // Culled with the camera
        vector<entt::entity> drawVec;

        // Global hashGrid for all entities
        SingleResolutionHashGrid<entt::entity, 32> hashGrid{200};

        // Collects entities
        HashSet<entt::entity> collector{500};

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
            collector.clear();
            hashGrid.clear();
        }
    };

    namespace global
    {
        inline LogicTickData LOGIC_TICK_DATA;
    }

} // namespace magique

#endif //LOGICTICKDATA_H