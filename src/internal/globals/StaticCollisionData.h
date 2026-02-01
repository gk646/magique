// SPDX-License-Identifier: zlib-acknowledgement
#ifndef STATIC_COLLISION_DATA_H
#define STATIC_COLLISION_DATA_H

#include <magique/internal/Macros.h>
#include <magique/util/Datastructures.h>

#include "internal/datastructures/MultiResolutionGrid.h"

//-----------------------------------------------
// Static Collision Data
//-----------------------------------------------
// .....................................................................
// Holds all data relevant to only static collision detection. Static collision consists of 4 major parts:
// 1. World Bounds: Set through the core/Core.h interface - defines static world bounds
// 2. TileMapObjects: Defined in the tile editor and imported
// 3. TileSet: Defined in the tile editor - tile of a tileset are marked with a number and imported
// 4. Custom Groups: Defined at runtime manually - identified by a group handle can be loaded and unloaded
// .....................................................................

namespace magique
{
    using StaticID = uint64_t; // 32 bits the object num - rest is data (collider class, tilenum, group num)

    struct StaticPair final // Caches information when the collision occurs to reuse later on
    {
        CollisionInfo info;    // Collision info
        entt::entity entity;   // the entity
        uint32_t objectNum;    // objectnum (static collider number)
        int data;              // Collider data
        ColliderType type;     // collider type
        EntityType entityType; // entity type - for the script
    };

    using ColliderHashGrid = SingleResolutionHashGrid<StaticID, MAGIQUE_MAX_ENTITIES_CELL, 64>; // power of two
    using TileHashGrid = SingleResolutionHashGrid<StaticID, MAGIQUE_MAX_ENTITIES_CELL, 32>;     // power of two
    using GroupHashGrid = SingleResolutionHashGrid<StaticID, MAGIQUE_MAX_ENTITIES_CELL, 32>;    // power of two

    using StaticPairCollector = AlignedVec<StaticPair>[MAGIQUE_WORKER_THREADS + 1];
    using ColliderCollector = AlignedVec<StaticID>[MAGIQUE_WORKER_THREADS + 1];

    struct ColliderStorage final
    {
        std::vector<StaticCollider> colliders;
        std::vector<uint32_t> freeList;

        [[nodiscard]] const StaticCollider& get(const uint32_t index) const { return colliders[index]; }

        uint32_t insert(const Rect& r)
        {
            if (freeList.empty())
            {
                const auto currIdx = colliders.size();
                colliders.emplace_back(r);
                return currIdx;
            }
            const auto nextIdx = freeList.back();
            freeList.pop_back();
            colliders[nextIdx].bounds = r;
            return nextIdx;
        }

        void remove(const uint32_t objectNum)
        {
            MAGIQUE_ASSERT(objectNum < colliders.size(), "Given num is out of bounds");
            auto& collider = colliders[objectNum];
            MAGIQUE_ASSERT(collider.bounds.size() != 0, "Attempting to delete a deleted collider");
            collider.bounds.zero();
            freeList.push_back(objectNum);
        }
    };

    struct StaticIDHelper final // Helps to get individual parts from the static ids
    {
        static uint32_t GetObjectNum(const StaticID id) { return static_cast<uint32_t>(id >> 32); }

        static int GetData(const StaticID id) { return static_cast<int32_t>(id & 0xFFFFFFFF); }

        static StaticID CreateID(const uint32_t objectNum, const int data)
        {
            const auto ret = (static_cast<StaticID>(objectNum) << 32) | static_cast<uint32_t>(data);
            MAGIQUE_ASSERT(GetData(ret) == data, "Wrong conversion");
            MAGIQUE_ASSERT(GetObjectNum(ret) == objectNum, "Wrong conversion");
            return ret;
        }
    };

    struct ObjectReferenceHolder final
    {
        struct TileObjectInfo final // Saves the pointer to the vec and the loaded ids
        {
            std::vector<uint32_t> objectIds;
            const void* vectorPointer = nullptr;
        };

        struct ManualGroupInfo final // Saves the id and the loaded ids
        {
            std::vector<uint32_t> objectIds;
            int groupId = -1;
        };

        // Maps + which colliders where loaded for each map (can be many for each map)
        HashMap<MapID, std::vector<TileObjectInfo>> tileObjectMap;
        // Tiles + what colliders where loaded per map
        HashMap<MapID, std::vector<uint32_t>> tilesCollisionMap;
        // What groups where loaded for ach map (can be many for each map)
        HashMap<MapID, std::vector<ManualGroupInfo>> groupMap;
    };

    struct StaticCollisionData final
    {
        Rectangle worldBounds{}; // World bounds

        //----------------- COLLISION SYSTEM  -----------------//
        StaticPairCollector pairCollector;     // Collects pairs for all types entity + (world, object, tiles, custom)
        ColliderCollector colliderCollector{}; // Collects collider ids

        //----------------- STORAGE-----------------//

        ColliderStorage colliderStorage;          // Holds all objects - uses a free list to preserve indices
        ObjectReferenceHolder colliderReferences; // Saves data about the static collision object so they can be removed

        //----------------- HASHGRIDS -----------------//

        MapHolder<ColliderHashGrid> mapObjectGrids; // Stores all tilemap objects
        MapHolder<TileHashGrid> mapTileGrids;       // Stores all collidable tiles
        MapHolder<GroupHashGrid> mapGroupGrids;     // Stores all objects from manual collider groups

        //----------------- TILESET -----------------//
        const TileSet* tileSet = nullptr; // Only use for equality checks
        float tileSetScale = 1.0f;
        HashMap<uint16_t, TileInfo> markedTilesMap; // which tiles are marked and their tile info

        [[nodiscard]] bool getIsWorldBoundSet() const { return worldBounds.width != 0 && worldBounds.height != 0; }
    };

    namespace global
    {
        inline StaticCollisionData STATIC_COLL_DATA{};
    }

} // namespace magique


#endif //STATIC_COLLISION_DATA_H
