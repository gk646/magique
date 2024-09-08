#ifndef STATIC_COLLISION_DATA_H
#define STATIC_COLLISION_DATA_H

#include <magique/internal/Macros.h>

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
    using StaticID = uint64_t; // 32 bits the object num - rest is data (collider class, tilenum, group num)

    struct StaticPair final // Caches information when the collision occurs to reuse later on
    {
        CollisionInfo info;    // Collision info
        CollisionC* col;       // the entities collision component
        entt::entity entity;   // the entity
        uint32_t objectNum;    // objectnum (static collider number)
        int data;              // Collider data
        ColliderType type;     // collider type
        EntityType entityType; // entity type - for the script
    };

    using ColliderHashGrid = SingleResolutionHashGrid<StaticID, 7, 128>; // power of two
    using TileHashGrid = SingleResolutionHashGrid<StaticID, 7, 32>;      // power of two
    using GroupHashGrid = SingleResolutionHashGrid<StaticID, 7, 64>;     // power of two

    using StaticPairCollector = AlignedVec<StaticPair>[MAGIQUE_WORKER_THREADS + 1];
    using ColliderCollector = AlignedVec<StaticID>[MAGIQUE_WORKER_THREADS + 1];

    struct ColliderStorage final
    {
        vector<StaticCollider> colliders;
        vector<uint32_t> freeList;

        uint32_t insert(const float x, const float y, const float width, const float height)
        {
            if (freeList.empty())
            {
                const auto currIdx = colliders.size();
                colliders.push_back({x, y, width, height});
                return currIdx;
            }
            const auto nextIdx = freeList.back();
            freeList.pop_back();
            colliders[nextIdx] = {x, y, width, height};
            return nextIdx;
        }
        void remove(const uint32_t objectNum)
        {
            MAGIQUE_ASSERT((int)objectNum < colliders.size(), "Given num is out of bounds");
            auto& collider = colliders[objectNum];
            MAGIQUE_ASSERT(collider.p1 != 0.0F || collider.p2 != 0.0F, "Attempting to delete a deleted collider");
            collider.p1 = 0;
            collider.p2 = 0;
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
        // Maps + which vectors where loaded
        HashMap<MapID, vector<uint32_t>> tileObjectMap;
        HashMap<MapID, std::array<const std::vector<TileObject>*, MAGIQUE_MAX_OBJECT_LAYERS>> objectVectors;
        // Tiles + which maps where loaded
        HashMap<MapID, vector<uint32_t>> markedTilesDataMap;
        // Groups
        HashMap<int, vector<uint32_t>> groupMap; // Holds manual collider groups
    };

    struct StaticCollisionData final
    {
        Rectangle worldBounds{}; // World bounds

        //----------------- COLLECTORS -----------------//
        StaticPairCollector pairCollector; // Collects all pairs for all types entity + (world, object, tiles, custom)
        ColliderCollector colliderCollector{}; // Collects collider ids

        //----------------- STORAGE-----------------//

        ColliderStorage objectStorage;          // Holds all objects - uses a free list to preserve indices
        HashSet<uint64_t> pairSet;              // Makes sure there are only unique collision paris (entity + object_id)
        ObjectReferenceHolder objectReferences; // Saves data so that inserted objects can automatically be unloaded

        //----------------- HASHGRIDS -----------------//

        ColliderHashGrid objectGrid; // Stores all tilemap objects
        TileHashGrid tileGrid;       // Stores all collidable tiles
        GroupHashGrid groupGrid;     // Stores all objects from manual collider groups

        //----------------- TILESET -----------------//
        const TileSet* tileSet = nullptr; // Only use for equality checks
        float tileSetScale = 1.0f;
        HashMap<uint16_t, TileInfo> markedTilesMap; // which tiles are marked and their tile info

        void unloadMap(const MapID map)
        {
            const auto clearGridAndVec = [&](auto& vec, auto& grid)
            {
                for (const auto num : vec)
                {
                    objectStorage.remove(num);
                    // We store the object num + the data in the hash grid
                    // But we dont want to store 8 byte in the objectReferences - and the object num is still unique
                    // So we compare the internal static id and see if the object num part matches
                    grid.removeIfWithHoles(num, [](const uint32_t num, const StaticID id)
                                           { return num == StaticIDHelper::GetObjectNum(id); });
                }
                grid.patchHoles(); // Patch as we cant rebuild it
                vec.clear();
            };

            if (objectReferences.tileObjectMap.contains(map))
            {
                clearGridAndVec(objectReferences.tileObjectMap[map], objectGrid);
                objectReferences.objectVectors.erase(map);
            }

            if (objectReferences.markedTilesDataMap.contains(map))
            {
                clearGridAndVec(objectReferences.markedTilesDataMap[map], tileGrid);
            }
        }

        [[nodiscard]] bool getIsWorldBoundSet() const { return worldBounds.width != 0 && worldBounds.height != 0; }
    };

    namespace global
    {
        inline StaticCollisionData STATIC_COLL_DATA{};
    }

} // namespace magique


#endif //STATIC_COLLISION_DATA_H