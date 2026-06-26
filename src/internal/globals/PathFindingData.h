// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PATHFINDING_DATA_H
#define MAGIQUE_PATHFINDING_DATA_H

#include <magique/core/Types.h>

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
        StaticDenseLookupGrid<bool, 200> visited{};
        StaticDenseLookupGrid<float, 200> openCost{};
        PriorityQueue<GridNode> frontier{500};
        GridNode nodePool[MAGIQUE_MAX_PATH_SEARCH_CAPACITY];

        // Lookup table for entity types and entities
        HashSet<Entity> solidEntities;
        HashSet<EntityType> solidTypes;

        //----------------- METHODS -----------------//

        // Checks if the given coordinates are in a solid tile - directly takes the grids to avoid the lookup
        static bool IsCellSolid(const float x, const float y, const PathFindingGrid& staticGrid,
                                const PathFindingGrid& dynamicGrid)
        {
            return staticGrid.getIsMarked(x, y) || dynamicGrid.getIsMarked(x, y);
        }

        [[nodiscard]] bool getIsPathSolid(const Entity e, const EntityType type) const
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

            // Add tileset tiles
            if (staticData.colliderReferences.tilesCollisionMap.contains(map))
            {
                const auto& objectIndices = staticData.colliderReferences.tilesCollisionMap.at(map);
                for (const auto idx : objectIndices)
                {
                    const auto& [x, y, w, h] = staticData.colliderStorage.get(idx).bounds;
                    rasterizeRect(x, y, w, h);
                }
            }
        }

        void initPathFinding(std::vector<Point>& path, const Point& start)
        {
            // Setup
            frontier.clear();
            path.clear();
            path.reserve(32);
            visited.setNewMid(start);
            openCost.setNewMid(start);
        }

        bool findPath(std::vector<Point>& path, Point start, Point end, const MapID map, const uint16_t maxPathLen,
                      GridMode mode, PathFindHeuristicFunc hFunc = nullptr)
        {
            start = Point{start / cellSize}.floor();
            end = Point{end / cellSize}.floor();
            initPathFinding(path, start);

            const auto& staticGrid = mapsStaticGrids[map];
            const auto& dynamicGrid = mapsDynamicGrids[map];

            uint16_t iteration = 0;
            uint16_t bestNodeIndex = 0;
            float bestDistance = 1e12;

            // Viability check
            if (IsCellSolid(start.x, start.y, staticGrid, dynamicGrid)) [[unlikely]]
            {
                return false;
            }

            if (hFunc == nullptr)
            {
                hFunc = PATH_HEURISTICS[(int)mode];
            }

            const PathFindMoveCostFunc mFunc = MOVE_COST[(int)mode];
            const auto& movement = MOVEMENTS[(int)mode];

            auto hCost = hFunc(start, end);
            frontier.emplace(start, 0.0F, hCost, hCost, UINT16_MAX, 0); // Initial node - no move cost
            while (!frontier.empty() && iteration < MAGIQUE_MAX_PATH_SEARCH_CAPACITY)
            {
                nodePool[iteration] = frontier.top();
                auto& current = nodePool[iteration];

                // Filter after distance as if we don't reach target don't care when taking longer route
                // Anything better than nothing
                const auto currentDistance = current.hCost;
                if (currentDistance < bestDistance) [[unlikely]]
                {
                    bestDistance = currentDistance;
                    bestNodeIndex = iteration;
                }

                if (current.position == end) [[unlikely]]
                {
                    constructPath(current, path);
                    return true;
                }

                frontier.pop();
                visited.setValue(current.position, true);

                if (current.stepCount < maxPathLen)
                {
                    for (const auto& dir : movement)
                    {
                        const auto newPosTile = current.position + dir;
                        const auto newPosWorld = newPosTile * cellSize;

                        // Is not visited and not solid
                        if (visited.getValue(newPosTile) ||
                            IsCellSolid(newPosWorld.x, newPosWorld.y, staticGrid, dynamicGrid))
                        {
                            continue;
                        }

                        const auto bestValueForTile = openCost.getValue(newPosTile);
                        const float gCost = mFunc(dir) + current.gCost;
                        const auto hCost = hFunc(newPosTile, end);
                        const auto newPathLen = static_cast<uint16_t>(current.stepCount + 1U);
                        const auto newFCost = hCost + gCost;

                        if (bestValueForTile != 0.0F && newFCost >= bestValueForTile)
                        {
                            continue;
                        }
                        frontier.push({newPosTile, gCost, newFCost, hCost, iteration, newPathLen});
                        openCost.setValue(newPosTile, newFCost);
                    }
                    iteration++;
                }
            }
            constructPath(nodePool[bestNodeIndex], path);
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

#endif // MAGIQUE_PATHFINDING_DATA_H
