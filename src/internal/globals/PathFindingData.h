// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PATHFINDING_DATA_H
#define MAGIQUE_PATHFINDING_DATA_H

#include <magique/core/Camera.h>
#include <magique/core/Types.h>

#include "external/cxstructs/cxstructs/PriorityQueue.h"
#include "internal/globals/StaticCollisionData.h"
#include "internal/utils/CollisionPrimitives.h"
#include "internal/datastructures/PathFindingStructs.h"

//-----------------------------------------------
// Pathfinding Data
//-----------------------------------------------
// .....................................................................
// Uses a stateless A* implementation with custom hashset and priority queue and octile distance heuristic
// Also weights the heuristics in favor of closing in on the target
// For collision lookups hashmaps are used with bitset to pack bit data
// There are two classes of solid objects: static and dynamic
//      - static : Static objects (TileObjects, TilSet, ...) see core/StaticCollision.h
//      - dynamic: Entities (defined by entityID, or by EntityType)
// .....................................................................

namespace magique
{
    using PathFindingGrid = DenseLookupGrid<MAGIQUE_PATHFINDING_CELL_SIZE>;

    struct PathFindingData final
    {
        // Constants
        static constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;

        // Grid data for each map - if cell is usable for pathfinding or not
        MapHolder<PathFindingGrid> mapsStaticGrids;
        MapHolder<PathFindingGrid> mapsDynamicGrids;

        // A star cache
        std::vector<Point> pathCache;
        StaticDenseLookupGrid<200> visited{};
        cxstructs::PriorityQueue<GridNode> frontier{};
        GridNode nodePool[MAGIQUE_MAX_PATH_SEARCH_CAPACITY];

        // Lookup table for entity types and entities
        HashSet<entt::entity> solidEntities;
        HashSet<EntityType> solidTypes;

        //----------------- METHODS -----------------//

        // Checks if the given coordinates are in a solid tile - directly takes the grids to avoid the lookup
        [[nodiscard]] static bool IsCellSolid(const float x, const float y, const PathFindingGrid& staticGrid,
                                              const PathFindingGrid& dynamicGrid)
        {
            if (staticGrid.getIsMarked(x, y)) [[unlikely]]
            {
                return true;
            }
            return dynamicGrid.getIsMarked(x, y);
        }

        [[nodiscard]] bool getIsPathSolid(const entt::entity e, const EntityType type) const
        {
            return solidTypes.contains(type) || solidEntities.contains(e);
        }

        // Updates the pathfinding grid for the given map
        void updateStaticPathGrid(const MapID map)
        {
            const auto& staticData = global::STATIC_COLL_DATA;
            auto& staticGrid = mapsStaticGrids[map];
            staticGrid.clear();

            const auto rasterizeRect = [&](const float x, const float y, const float w, const float h)
            {
                const int startX = static_cast<int>(std::floor(x / cellSize));
                const int startY = static_cast<int>(std::floor(y / cellSize));
                const int endX = static_cast<int>(std::floor((x + w) / cellSize));
                const int endY = static_cast<int>(std::floor((y + h) / cellSize));

                // Loop through potentially intersecting grid cells
                for (int i = startY; i <= endY; ++i)
                {
                    const auto cellY = static_cast<float>(i) * cellSize;
                    for (int j = startX; j <= endX; ++j)
                    {
                        const auto cellX = static_cast<float>(j) * cellSize;
                        // Check for intersection and mark the grid cell
                        if (RectToRect(x, y, w, h, cellX, cellY, cellSize, cellSize))
                        {
                            staticGrid.setMarked(cellX, cellY);
                        }
                    }
                }
            };

            // Add world bounds
            if (staticData.getIsWorldBoundSet())
            {
                constexpr float depth = MAGIQUE_WORLD_BOUND_DEPTH;
                const auto wBounds = staticData.worldBounds;
                const Rectangle r1 = {wBounds.x - depth, wBounds.y - depth, depth, wBounds.height + depth};
                const Rectangle r2 = {wBounds.x, wBounds.y - depth, wBounds.width, depth};
                const Rectangle r3 = {wBounds.x + wBounds.width, wBounds.y - depth, depth, wBounds.height + depth};
                const Rectangle r4 = {wBounds.x, wBounds.y + wBounds.height, wBounds.width, depth};
                rasterizeRect(r1.x, r1.y, r1.width, r1.height);
                rasterizeRect(r2.x, r2.y, r2.width, r2.height);
                rasterizeRect(r3.x, r3.y, r3.width, r3.height);
                rasterizeRect(r4.x, r4.y, r4.width, r4.height);
            }

            // Add tile objects
            if (staticData.colliderReferences.tileObjectMap.contains(map))
            {
                const auto& tileObjectInfo = staticData.colliderReferences.tileObjectMap.at(map);
                for (const auto& info : tileObjectInfo)
                {
                    for (const auto idx : info.objectIds)
                    {
                        const auto& [x, y, w, h] = staticData.colliderStorage.get(idx);
                        rasterizeRect(x, y, w, h);
                    }
                }
            }

            // Add tileset tiles
            if (staticData.colliderReferences.tilesCollisionMap.contains(map))
            {
                const auto& objectIndices = staticData.colliderReferences.tilesCollisionMap.at(map);
                for (const auto idx : objectIndices)
                {
                    const auto& [x, y, w, h] = staticData.colliderStorage.get(idx);
                    rasterizeRect(x, y, w, h);
                }
            }

            if (staticData.colliderReferences.groupMap.contains(map))
            {
                const auto& groupInfoVec = staticData.colliderReferences.groupMap.at(map);
                for (const auto& groupInfo : groupInfoVec)
                {
                    for (const auto idx : groupInfo.objectIds)
                    {
                        const auto& [x, y, w, h] = staticData.colliderStorage.get(idx);
                        rasterizeRect(x, y, w, h);
                    }
                }
            }
        }

        bool findPath(std::vector<Point>& path, const Point startC, const Point endC, const MapID map,
                      const uint16_t maxPathLen, GridMode mode)
        {
            const Point start = {std::floor(startC.x / cellSize), std::floor(startC.y / cellSize)};
            const Point end = {std::floor(endC.x / cellSize), std::floor(endC.y / cellSize)};

            // Setup
            uint16_t iterations = 0;
            frontier.clear();
            path.clear();
            path.reserve(maxPathLen);
            visited.clear();
            visited.setMid(start);
            const auto& staticGrid = mapsStaticGrids[map];
            const auto& dynamicGrid = mapsDynamicGrids[map];

            // Viability check
            if (IsCellSolid(end.x * cellSize, end.y * cellSize, staticGrid, dynamicGrid)) [[unlikely]]
            {
                return false;
            }

            const PathFindingHeuristicFunc hFunc = PATH_HEURISTICS[(int)mode];
            const PathFindingMoveCostFunc mFunc = MOVE_COST[(int)mode];
            const auto& movement = MOVEMENTS[(int)mode];

            frontier.emplace(start, 0.0F, hFunc(start, end), UINT16_MAX, 0); // Initial node
            while (!frontier.empty() && iterations < MAGIQUE_MAX_PATH_SEARCH_CAPACITY)
            {
                nodePool[iterations] = frontier.top();
                auto& current = nodePool[iterations];
                if (current.position == end)
                {
                    constructPath(current, path);
                    return true;
                }
                if (current.stepCount >= maxPathLen)
                {
                    return false;
                }

                frontier.pop();
                visited.setMarked(current.position.x, current.position.y);
                for (const auto& dir : movement)
                {
                    Point const newPos = {current.position.x + dir.x, current.position.y + dir.y};
                    const auto newPosCoX = newPos.x * cellSize;
                    const auto newPosCoY = newPos.y * cellSize;
                    // Is not visited and not solid
                    if (!visited.getIsMarked(newPos.x, newPos.y) &&
                        !IsCellSolid(newPosCoX, newPosCoY, staticGrid, dynamicGrid))
                    {
                        const float moveCost = mFunc(dir);
                        const auto hCost = hFunc(newPos, end);
                        const auto newPathLen = static_cast<uint16_t>(current.stepCount + 1U);
                        frontier.update({newPos, current.gCost + moveCost, hCost, iterations, newPathLen});
                    }
                }
                iterations++;
            }
            return false;
        }

    private:
        void constructPath(const GridNode& current, std::vector<Point>& path) const
        {
            const GridNode* curr = &current;
            while (curr->parent != UINT16_MAX)
            {
                const Point p = {(curr->position.x * cellSize) + (cellSize / 2.0F),
                                 (curr->position.y * cellSize) + (cellSize / 2.0F)};
                path.push_back(p);
                curr = &nodePool[curr->parent];
            }
        }
    };

    namespace global
    {
        inline PathFindingData PATH_DATA{};
    }
} // namespace magique

#endif //MAGIQUE_PATHFINDING_DATA_H