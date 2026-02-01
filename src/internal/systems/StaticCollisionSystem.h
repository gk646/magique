// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STATIC_COLLISION_SYSTEM_H
#define MAGIQUE_STATIC_COLLISION_SYSTEM_H
#include "magique/core/CollisionDetection.h"

//-----------------------------------------------
// Static Collision System
//-----------------------------------------------
// .....................................................................
// World bounds is given as white list area -> check against the outer rectangles
// Collidable tiles are treated as squares and inserted into the grid
//
// 1. Get all objects from the grid cells that intersect the entity bounding box
// 2. Calculate all the collision and sort them after distance from collision point to entity middle
// 3. Apply the resolution vec with the length of the pen depth
//      3.1. Avoid the sticky edges by skipping collisions iff:
//          - the entity has at least 1 other collision (before)
//          - the collision has
// .....................................................................

namespace magique
{
    void CheckStaticCollisionRange(int thread, int start, int end);
    void HandleCollisionPairs(StaticPairCollector& pairColl, HashSet<uint64_t>& pairSet);

    inline void StaticCollisionSystem()
    {
        const auto& data = global::ENGINE_DATA;
        auto& staticData = global::STATIC_COLL_DATA;
        const int size = data.collisionVec.size(); // Multithread over certain amount
        if (size < 100)
        {
            CheckStaticCollisionRange(0, 0, size);
        }
        else
        {
            std::array<jobHandle, COL_WORK_PARTS> handles{};
            int end = 0;
            const int partSize = static_cast<int>(static_cast<float>(size) * 0.81F) / COL_WORK_PARTS;
            for (int j = 0; j < COL_WORK_PARTS - 1; ++j) // Gives more work to main thread cause its faster
            {
                const int start = end;
                end = start + partSize;
                handles[j] = AddJob(CreateExplicitJob(CheckStaticCollisionRange, j, start, end));
            }
            CheckStaticCollisionRange(COL_WORK_PARTS - 1, end, size);
            AwaitJobs(handles); // Await completion - for caller its sequential -> easy reasoning and simplicity
        }
        // Handle unique pairs - we can share the pair set with dynamic
        HandleCollisionPairs(staticData.pairCollector, global::DY_COLL_DATA.pairSet);
    }

    inline void CheckAgainstWorldBounds(std::vector<StaticPair>& collector, const entt::entity e, const PositionC& pos,
                                        const CollisionC& col, const Rectangle& r, const uint32_t num)
    {
        CollisionInfo info{};
        CheckCollisionEntityRect(pos, col, r, info);
        if (info.isColliding())
        {
            // subtract 0-3 depending on the world bound
            // We just need to have a unique object num so if a collision is found in multiple cells
            // due to duplicate insertion it's not processed
            // UINT32_MAX will never be reached and subtracting will be never be negative
            collector.push_back({info, e, UINT32_MAX - num, 0, ColliderType::WORLD_BOUNDS, pos.type});
        }
    }

    template <class TypeHashGrid>
    void CheckHashGrid(const entt::entity e, const TypeHashGrid& grid, std::vector<StaticID>& collector,
                    std::vector<StaticPair>& pairCollector, const ColliderType type,
                       const std::vector<StaticCollider>& colliders, const PositionC& pos, const CollisionC& col)
    {
        const auto bb = GetEntityBoundingBox(pos, col);
        grid.query(collector, bb.x, bb.y, bb.width, bb.height);
        for (const auto num : collector)
        {
            const auto objectNum = StaticIDHelper::GetObjectNum(num);
            CollisionInfo info{};
            CheckCollisionEntityRect(pos, col, colliders[(int)objectNum].bounds, info);
            if (info.isColliding())
            {
                pairCollector.push_back({info, e, objectNum, StaticIDHelper::GetData(num), type, pos.type});
            }
        }
        collector.clear();
    }

    inline void CheckStaticCollisionRange(const int thread, const int start, const int end) // Runs on each thread
    {

        const auto& data = global::ENGINE_DATA;
        const auto& group = internal::POSITION_GROUP;
        auto& staticData = global::STATIC_COLL_DATA; // non const - modifying the collectors

        const auto& collisionVec = data.collisionVec;
        const auto& colliderStorage = staticData.colliderStorage.colliders;

        // Just use for the hash grid queries
        auto& idCollector = staticData.colliderCollector[thread].vec; // non const
        // Used to store the info if a collision occurs
        auto& pairCollector = staticData.pairCollector[thread].vec; // non const

        // Cache
        constexpr float depth = 250.0F;
        const auto wBounds = staticData.worldBounds;
        const Rectangle r1 = {wBounds.x - depth, wBounds.y - depth, depth, wBounds.height + depth};
        const Rectangle r2 = {wBounds.x, wBounds.y - depth, wBounds.width, depth};
        const Rectangle r3 = {wBounds.x + wBounds.width, wBounds.y - depth, depth, wBounds.height + depth};
        const Rectangle r4 = {wBounds.x, wBounds.y + wBounds.height, wBounds.width, depth};
        const auto checkWorld = staticData.getIsWorldBoundSet();

        const auto startIt = collisionVec.begin() + start;
        const auto endIt = collisionVec.begin() + end;
        for (auto it = startIt; it != endIt; ++it)
        {
            const auto e = *it;
            const auto& pos = group.get<const PositionC>(e);
            const auto& col = group.get<CollisionC>(e); // Non cost for saving modifiable pointer

            if (checkWorld) // Check if worldbounds active
            {
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r1, 0);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r2, 1);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r3, 2);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r4, 3);
            }
            const auto map = pos.map;

            // Query object grid if it has any entries
            if (staticData.mapObjectGrids.contains(map))
            {
                const auto& objectGrid = staticData.mapObjectGrids[map];
                constexpr auto objectType = ColliderType::TILEMAP_OBJECT;
                CheckHashGrid(e, objectGrid, idCollector, pairCollector, objectType, colliderStorage, pos, col);
            }

            // Query tile grid
            if (staticData.mapTileGrids.contains(map))
            {
                const auto& tileGrid = staticData.mapTileGrids[map];
                constexpr auto tileType = ColliderType::TILESET_TILE;
                CheckHashGrid(e, tileGrid, idCollector, pairCollector, tileType, colliderStorage, pos, col);
            }

            // Query group grid
            if (staticData.mapGroupGrids.contains(map))
            {
                const auto& groupGrid = staticData.mapGroupGrids[map];
                constexpr auto groupType = ColliderType::MANUAL_COLLIDER;
                CheckHashGrid(e, groupGrid, idCollector, pairCollector, groupType, colliderStorage, pos, col);
            }
        }
    }

    inline void HandleCollisionPairs(StaticPairCollector& pairColl, HashSet<uint64_t>& pairSet)
    {
        const auto& scripts = global::SCRIPT_DATA.scripts;
        auto& dynamic = global::DY_COLL_DATA;
        for (auto& [vec] : pairColl)
        {
            for (auto& [info, e, objNum, data, objType, entType] : vec)
            {
                // This cannot be avoided as duplicates are inserted into the hashgrid
                if (dynamic.isMarked(e, objNum))
                {
                    continue;
                }

                // Also check existence
                auto* col = ComponentTryGet<CollisionC>(e);
                if (col == nullptr)
                {
                    continue;
                }

                // Process the collision
                const auto colliderInfo = ColliderInfo{data, objType};
                ScriptingInvokeEventDirect<onStaticCollision>(scripts[entType], e, colliderInfo, info);

                if (info.getIsAccumulated()) // Accumulate the data if specified
                {
                    AccumulateInfo(*col, Shape::RECT, info);
                }
            }
            vec.clear();
        }
        dynamic.pairSet.clear();
    }
} // namespace magique


#endif //MAGIQUE_STATIC_COLLISION_SYSTEM_H
