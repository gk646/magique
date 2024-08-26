#ifndef STATICCOLLISIONDATA_H
#define STATICCOLLISIONDATA_H

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/HashTypes.h"
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
    struct StaticPair final
    {
        CollisionInfo info;
        CollisionC* col;     // the entities collision
        entt::entity entity; // the entity
        uint16_t data;       // collider data
        ColliderType type;   // collider type
    };

    struct MapData final
    {
        const uint16_t* layerData[MAGIQUE_MAX_TILE_LAYERS]{};
        uint16_t width = 0;
        uint16_t height = 0;
        uint8_t layers = 0;
    };

    struct ObjectHolder final
    {
        vector<StaticCollider> colliders;
        vector<uint16_t> freeList;

        uint16_t insert(const float x, const float y, const float width, const float height)
        {
            if (freeList.empty())
            {
                const auto currIdx = static_cast<uint16_t>(colliders.size());
                colliders.push_back({x, y, width, height});
                MAGIQUE_ASSERT(currIdx < UINT16_MAX, "Too many static colliders");
                return currIdx;
            }
            const auto nextIdx = freeList.back();
            freeList.pop_back();
            colliders[nextIdx] = {x, y, width, height};
            return nextIdx;
        }
        uint16_t remove(const float x, const float y, const float width, const float height)
        {
            uint16_t i = 0;
            for (auto& col : colliders)
            {
                if (col.x == x && col.y == y && col.p1 == width && col.p2 == height)
                {
                    col.p1 = 0;
                    col.p2 = 0;
                    freeList.push_back(i);
                    return i;
                }
                ++i;
                MAGIQUE_ASSERT(i < UINT16_MAX, "Too many static colliders");
            }
            return UINT16_MAX;
        }
    };

    using StaticID = uint64_t; // 16 bits objectid (maps number to dims), 16 bits data (tilenum, groupnum),  rest is type

    struct StaticIDHelper final
    {
        // Extract the ColliderType from the upper 32 bits
        static ColliderType GetType(const StaticID id) { return static_cast<ColliderType>(id >> 32); }

        // Extract the data from the middle 16 bits
        static uint16_t GetData(const StaticID id) { return static_cast<uint16_t>(id >> 16 & 0xFFFF); }

        // Extract the object number from the lower 16 bits
        static uint16_t GetObjectNum(const StaticID id) { return static_cast<uint16_t>(id & 0xFFFF); }

        static StaticID CreateID(const uint16_t objectNum, const uint16_t data, const ColliderType type)
        {
            return static_cast<StaticID>(type) << 32 | static_cast<StaticID>(data) << 16 |
                static_cast<StaticID>(objectNum);
        }
    };

    using ColliderHashGrid = SingleResolutionHashGrid<StaticID, 14, 128>; // power of two
    using StaticPairCollector = AlignedVec<StaticPair>[WORK_PARTS];
    using ColliderCollector = AlignedVec<uint16_t>[WORK_PARTS];

    struct StaticCollisionData final
    {
        StaticPairCollector pairCollector; // Collects all pairs for all types entity + (world, object, tiles, custom)
        HashSet<uint64_t> pairSet;         // Makes sure there are only unique collision paris

        //----------------- COLLISION OBJECTS -----------------//

        ColliderHashGrid objectGrid;                               // Stores all objects and tiles
        HashMap<MapID, const std::vector<TileObject>*> mapObjects; // Saves pointer to object vector to unload later
        ObjectHolder objectHolder;             // Saves collider ids - uses a free list to preserve indices
        ColliderCollector colliderCollector{}; // Collects collider ids

        //----------------- TILESET -----------------//

        // hashset vs vec vs custom bitset
        vector<bool> tileCollisionVec;    // Holds information if the tile-index is solid
        vector<MapData> mapDataVec;       // Holds the begin pointers to each map
        const TileSet* tileSet = nullptr; // Only used for equality checks - global tileset
        float tileSize = 16.0F;

        //----------------- MANUAL COLLIDERS -----------------//

        HashMap<int, vector<StaticCollider>> manualColliderMap; // HashMap, as group can be any number

        [[nodiscard]] bool isSolid(const uint16_t tileNum) const
        {
            return tileNum < tileCollisionVec.size() && tileCollisionVec[tileNum];
        }

        void unloadMap(const MapID map)
        {
            const auto it = mapObjects.find(map);
            if (it == mapObjects.end())
                return;

            for (const auto& obj : *it->second)
            {
                const auto num = objectHolder.remove(obj.x, obj.y, obj.width, obj.height);
                objectGrid.removeWithHoles(num);
            }
            objectGrid.patchHoles(); // patch any holes - for static collision we dont rebuild the grid
        }

        [[nodiscard]] const StaticCollider& getCollider(const uint16_t id) const { return objectHolder.colliders[id]; }
    };

    namespace global
    {
        inline StaticCollisionData STATIC_COLL_DATA{};
    }

} // namespace magique


#endif //STATICCOLLISIONDATA_H