// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PATHFINDING_DATA_H
#define MAGIQUE_PATHFINDING_DATA_H

#include <bitset>
#include <magique/core/Camera.h>

#include "external/cxstructs/cxstructs/PriorityQueue.h"
#include "internal/globals/StaticCollisionData.h"
#include "internal/utils/CollisionPrimitives.h"

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
    struct GridNode final
    {
        Point position{};
        float fCost = 0.0F;
        float gCost = 0.0F;
        uint16_t parent = UINT16_MAX;
        GridNode() = default;
        GridNode(const Point position, const float gCost, const float hCost, const uint16_t parent) :
            position(position), fCost(gCost + hCost), gCost(gCost), parent(parent)
        {
        }
        bool operator>(const GridNode& o) const { return fCost > o.fCost; }
    };

    using VisitedCellID = uint32_t;

    [[nodiscard]] static VisitedCellID GetVisitedCell(const int cellX, const int cellY)
    {
        // VisitedCellID must be uint otherwise the shifting doesn't work
        // A signed int has its signed saved in the most significant bit regardless of size
        // Thus shifting the number left shifts it away...
        // The casting of negative numbers to uint results in: Largest value - abs(value)
        // So negative numbers go down from the top - which is good for hash distribution anyway
        const auto first = static_cast<uint16_t>(cellX);
        const auto second = static_cast<uint16_t>(cellY);
        return (static_cast<VisitedCellID>(first) << 16) | second;
    }

    // This is just a technique to pack data more closely
    // Instead of treating the pathfinding grid as a grid with the main size given in config.h
    // We increase the outer grid by a factor - this reduces the values stored in the hashmap which is already better.
    // Then we use a bitset to tightly pack the data of each of the subgrids within the bigger grid
    // Given a MAGIQUE_PATHFINDING_CELL_SIZE of 32 and a subgrid size of 16 the outer grid is 16 * 32 = 512
    // This means there are 16 * 16 normal grids inside the enlarged grid
    // By dividing the normalize coordinate inside the current enlarged grid cell (value between 0 - 512) by 32
    // We get the index at which it is stored inside the bitset (flattened array)
    template <int mainGridBaseSize, int subGridSize = 16> // Fits into cache line (key/value pair)
    struct DenseLookupGrid final
    {
        // Is constexpr and power of 2 to get optimized division and modulo
        constexpr static int mainGridSize = mainGridBaseSize * subGridSize;

        HashMap<VisitedCellID, std::bitset<subGridSize * subGridSize>> visited{};

        [[nodiscard]] bool getIsMarked(const float x, const float y) const
        {
            // < -1 0 < 1
            const int cellX = floordiv<mainGridSize>(static_cast<int>(x));
            const int cellY = floordiv<mainGridSize>(static_cast<int>(y));
            const auto pathCell = GetVisitedCell(cellX, cellY);
            const auto it = visited.find(pathCell);
            if (it != visited.end()) // Get the position within the subgrid
            {
                const int vCellX = abs(static_cast<int>(x)) % mainGridSize / mainGridBaseSize;
                const int vCellY = abs(static_cast<int>(y)) % mainGridSize / mainGridBaseSize;
                return it->second[vCellX + (vCellY * subGridSize)];
            }
            return false;
        }

        void setMarked(const float x, const float y)
        {
            const int cellX = floordiv<mainGridSize>(static_cast<int>(x));
            const int cellY = floordiv<mainGridSize>(static_cast<int>(y));
            const int vCellX = abs(static_cast<int>(x)) % mainGridSize / mainGridBaseSize;
            const int vCellY = abs(static_cast<int>(y)) % mainGridSize / mainGridBaseSize;
            const auto cell = GetVisitedCell(cellX, cellY);
            visited[cell].set(vCellX + (vCellY * subGridSize), true);
        }

        void insert(const float x, const float y, const float w, const float h)
        {
            auto insertFunc = [this](const int cellX, const int cellY)
            { setMarked((cellX * MAGIQUE_PATHFINDING_CELL_SIZE), (cellY * MAGIQUE_PATHFINDING_CELL_SIZE)); };
            RasterizeRect<mainGridBaseSize>(insertFunc, x, y, w, h);
        }

        void clear() { visited.clear(); }
    };

    // Lookup grid that takes all given positions relative to its center
    // This works good inside a single search - cleared between each search
    // Allows very fast lookups without a hashmap
    template <int size>
    struct StaticDenseLookupGrid final
    {
        std::array<bool, size> rows[size]{};
        int midX;
        int midY;

        void setMid(const Point& mid)
        {
            midX = static_cast<int>(mid.x);
            midY = static_cast<int>(mid.y);
        }

        [[nodiscard]] bool getIsMarked(const float x, const float y) const
        {
            const auto relX = static_cast<int>(x) - midX + size / 2;
            const auto relY = static_cast<int>(y) - midY + size / 2;
            if (relX >= 0 && relX < size && relY >= 0 && relY < size) [[likely]]
            {
                return rows[relY][relX];
            }
            return true;
        }

        void setMarked(const float x, const float y)
        {
            const auto relX = static_cast<int>(x) - midX + size / 2;
            const auto relY = static_cast<int>(y) - midY + size / 2;
            if (relX >= 0 && relX < size && relY >= 0 && relY < size) [[likely]]
            {
                rows[relY][relX] = true;
            }
        }

        void clear() { memset(rows, 0, size * size * sizeof(bool)); }
    };

    using PathFindingGrid = DenseLookupGrid<MAGIQUE_PATHFINDING_CELL_SIZE>;

    struct PathFindingData final
    {
        // Constants
        static constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
        static constexpr Point crossMove[4] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};

        // Grid data for each map - if cell is usable for pathfinding or not
        MapHolder<PathFindingGrid> mapsStaticGrids;
        MapHolder<PathFindingGrid> mapsDynamicGrids;

        // A star cache
        std::vector<Point> pathCache;
        StaticDenseLookupGrid<200> visited{};
        StaticDenseLookupGrid<200> added{};
        cxstructs::PriorityQueue<GridNode> frontier{};
        GridNode nodePool[MAGIQUE_PATHFINDING_SEARCH_CAPACITY];

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
            if (!mapsStaticGrids.contains(map))
                mapsStaticGrids.add(map);
            if (!mapsDynamicGrids.contains(map))
                mapsDynamicGrids.add(map);

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

        //TODO can be optimized by using bidirectional search
        bool findPath(std::vector<Point>& path, const Point startC, const Point endC, const MapID map,
                      const uint16_t maxLen)
        {
            const Point start = {std::floor(startC.x / cellSize), std::floor(startC.y / cellSize)};
            const Point end = {std::floor(endC.x / cellSize), std::floor(endC.y / cellSize)};

            // Setup and get grids
            frontier.clear();
            path.clear();
            path.reserve(maxLen);

            visited.clear();
            visited.setMid(start);

            added.clear();
            added.setMid(start);

            const auto& staticGrid = mapsStaticGrids[map];
            const auto& dynamicGrid = mapsDynamicGrids[map];

            // Viability check
            if (IsCellSolid(end.x * cellSize, end.y * cellSize, staticGrid, dynamicGrid))
            {
                return false;
            }

            // Weighted towards goal
            auto targetHeuristic = [](const Point& curr, const Point& end) { return curr.octile(end) * 1.15F; };

            frontier.emplace(start, 0.0F, targetHeuristic(start, end), UINT16_MAX);
            added.setMarked(start.x, start.y);
            uint16_t counter = 0;

            while (!frontier.empty() && counter < maxLen)
            {
                nodePool[counter] = frontier.top();
                auto& current = nodePool[counter];
                if (current.position == end)
                {
                    constructPath(current, path);
                    return true;
                }
                frontier.pop();
                visited.setMarked(current.position.x, current.position.y);

                /*
                auto cam = GetCamera();
                DrawRectangleRec({current.position.x * cellSize - cam.target.x + cam.offset.x,
                                  current.position.y * cellSize - cam.target.y + cam.offset.y, cellSize, cellSize},
                                 PURPLE);
                */

                for (const auto& dir : crossMove)
                {
                    Point const newPos = {current.position.x + dir.x, current.position.y + dir.y};
                    const auto newPosCoX = newPos.x * cellSize;
                    const auto newPosCoY = newPos.y * cellSize;
                    // Is not visited and not solid
                    if (!added.getIsMarked(newPos.x, newPos.y) && !visited.getIsMarked(newPos.x, newPos.y) &&
                        !IsCellSolid(newPosCoX, newPosCoY, staticGrid, dynamicGrid))
                    {
                        const float moveCost = dir.x != 0 && dir.y != 0 ? 1.41F : 1.0F;
                        const auto hCost = targetHeuristic(newPos, end);
                        frontier.push({newPos, current.gCost + moveCost, hCost, counter});
                        added.setMarked(newPos.x, newPos.y);
                    }
                }
                counter++;
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