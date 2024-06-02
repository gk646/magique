#ifndef INTERNALTYPES_H
#define INTERNALTYPES_H

#include <magique/core/Types.h>
#include <magique/core/Defines.h>
#include <cxstructs/BitMask.h>
#include <ankerl/unordered_dense.h>

#include "core/datastructures/HashGrid.h"
#include "core/datastructures/QuadTree.h"
#include "core/datastructures/MultiResolutionGrid.h"

using CollisionPair = std::pair<entt::entity, entt::entity>;
struct PairHash
{
    std::size_t operator()(const CollisionPair& p) const
    {
        const uint64_t combined = (static_cast<uint64_t>(p.first) << 32) | static_cast<uint64_t>(p.second);
        return std::hash<uint64_t>()(combined);
    }
};

namespace magique
{
    struct LogicTickData final
    {
        const Map* currentMap = nullptr;
        Pint cameraTilePos{};
        Point cameraPos{};
        MapID currentZone;
        entt::entity camera;
        MapID loadedZones[MAGQIQUE_MAX_PLAYERS];

        // Change set for multiplayer events
        HashMap<entt::entity, cxstructs::EnumMask<UpdateFlag>> changedSet;

        // For how long entities in the cache are still updates after they are out of range
        uint16_t entityCacheDuration = 300; // 300 Ticks -> 5 seconds

        // Cache for entities that are not in update range anymore
        // They are still updated for cache duration
        HashMap<entt::entity, uint16_t> entityUpdateCache;

        // vector containing the entites to update for this ticka
        vector<entt::entity> entityUpdateVec;

        // vector containing all entites to be drawn this tick
        // Culled with the camera
        vector<entt::entity> drawVec;

        // Contains entities that have been removed
        // This is for multiplayer queued updates
        vector<entt::entity> removedEntities;

        // Checked collision pairs for each tick
        HashSet<CollisionPair, PairHash> checkedPairs;

        // Global hashGrid for all entities
        SingleResolutionHashGrid<entt::entity,32> hashGrid{100};

        // Global quadtree
        QuadTree<entt::entity> quadTree{32000, 32000};

        vector<entt::entity> collector;

        // Atomic spinlock - whenever and data is accessed on the draw thread
        std::atomic_flag flag;

        LogicTickData()
        {
            hashGrid.reserve(50,1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
            removedEntities.reserve(100);

            //Collision pairs
            checkedPairs.reserve(1000);

            //MP Update set
            changedSet.reserve(1000);

            //Update cache
            entityUpdateCache.reserve(1000);
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
            removedEntities.clear();
            checkedPairs.clear();
        }
    };
} // namespace magique
#endif //INTERNALTYPES_H