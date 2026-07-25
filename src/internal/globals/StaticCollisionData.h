// SPDX-License-Identifier: zlib-acknowledgement
#ifndef STATIC_COLLISION_DATA_H
#define STATIC_COLLISION_DATA_H


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
    // 32 bits the object num - rest is data (collider class, tilenum, group num)
    struct StaticID
    {
        uint32_t idx; // Index into collider storage
        int data;
    };

    struct StaticPair final // Caches information when the collision occurs to reuse later on
    {
        CollisionInfo info;    // Collision info
        Entity entity;         // the entity
        uint32_t objectNum;    // objectnum (static collider number)
        int data;              // Collider data
        ColliderType type;     // collider type
        EntityType entityType; // entity type - for the script
    };

    using TileHashGrid = SingleResolutionHashGrid<StaticID, MAGIQUE_MAX_ENTITIES_CELL, 32>;
    using StaticPairCollector = AlignedVec<StaticPair>[MAGIQUE_WORKER_THREADS + 1];
    using ColliderCollector = AlignedVec<StaticID>[MAGIQUE_WORKER_THREADS + 1];

    struct ColliderStorage final
    {
        std::vector<StaticCollider> colliders;
        std::vector<uint32_t> freeList;

        const StaticCollider& operator[](const uint32_t index) const { return colliders[index]; }

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
            collider.bounds = Rect{};
            freeList.push_back(objectNum);
        }
    };

    struct ObjectReferenceHolder final
    {
        // Tiles + what colliders where loaded per map
        HashMap<MapID, std::vector<uint32_t>> tilesCollisionMap;
    };

    struct StaticCollisionData final
    {
        Rectangle worldBounds{};                  // World bounds
        StaticPairCollector pairCollector;        // Collects pairs for all types entity + (world, object, tiles, custom)
        ColliderCollector colliderCollector{};    // Collects collider ids
        ColliderStorage colliderStorage;          // Holds all objects - uses a free list to preserve indices
        ObjectReferenceHolder colliderReferences; // Saves data about the static collision object so they can be removed
        MapHolder<TileHashGrid> mapTileGrids;     // Stores all collidable tiles
        const TileSet* tileSet = nullptr;         // Only use for equality checks
        float tileSetScale = 1.0f;
        HashMap<uint16_t, TileInfo> markedTilesMap; // which tiles are marked and their tile info

        [[nodiscard]] bool getIsWorldBoundSet() const { return worldBounds.width != 0 && worldBounds.height != 0; }
    };

    namespace global
    {
        inline StaticCollisionData STATIC_COLL_DATA{};
    }

} // namespace magique


#endif // STATIC_COLLISION_DATA_H
