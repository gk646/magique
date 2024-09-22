#ifndef MAGIQUE_PATHFINDING_DATA_H
#define MAGIQUE_PATHFINDING_DATA_H

#include <vector>
#include <bitset>

#include <magique/ecs/ECS.h>
#include <magique/util/Logging.h>

#include "external/cxstructs/cxstructs/PriorityQueue.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/StaticCollisionData.h"
#include "internal/globals/DynamicCollisionData.h"
#include "internal/datastructures/HashTypes.h"
#include "internal/headers/CollisionPrimitives.h"

//-----------------------------------------------
// Pathfinding Data
//-----------------------------------------------
// .....................................................................
// Uses a stateless A star implementation with custom hashset and priority queue and manhattan distance heuristic
// For collision lookups hashmaps are used with bitset to pack bit data
// .....................................................................

namespace magique
{
    struct GridNode final
    {
        Point position;
        float fCost;
        float gCost;
        uint16_t parent;
        GridNode() = default;
        GridNode(const Point position, const float gCost, const float hCost, const uint16_t parent) :
            position(position), fCost(gCost + hCost), gCost(gCost), parent(parent)
        {
        }
        bool operator>(const GridNode& o) const { return fCost > o.fCost; }
    };

    template <int mainGridBaseSize, int subGridSize = 16> // Fits into cache line (key/value pair)
    struct DenseLookupGrid final
    {
        constexpr static int mainGridSize = mainGridBaseSize * subGridSize;

        using VisitedCellID = uint32_t;

        static [[nodiscard]] VisitedCellID GetVisitedCell(const int cellX, const int cellY)
        {
            const auto first = static_cast<uint16_t>(cellX);
            const auto second = static_cast<uint16_t>(cellY);
            return static_cast<VisitedCellID>(first) << sizeof(VisitedCellID) * 4 | second;
        }

        HashMap<VisitedCellID, std::bitset<subGridSize * subGridSize>> visited{};

        [[nodiscard]] bool getIsMarked(const float x, const float y) const
        {
            const int cellX = static_cast<int>(std::floor(x / mainGridSize));
            const int cellY = static_cast<int>(std::floor(y / mainGridSize));
            const auto pathCell = GetVisitedCell(cellX, cellY);
            const auto it = visited.find(pathCell);
            if (it != visited.end())
            {
                const int vCellX = (static_cast<int>(x) - cellX * mainGridSize) / subGridSize;
                const int vCellY = (static_cast<int>(y) - cellY * mainGridSize) / subGridSize;
                return it->second[vCellX + vCellY * subGridSize];
            }
            return false;
        }

        void setMarked(const float x, const float y)
        {
            const int cellX = static_cast<int>(std::floor(x / mainGridSize));
            const int cellY = static_cast<int>(std::floor(y / mainGridSize));
            const int vCellX = (static_cast<int>(x) - cellX * mainGridSize) / subGridSize;
            const int vCellY = (static_cast<int>(y) - cellY * mainGridSize) / subGridSize;
            visited[GetVisitedCell(cellX, cellY)].set(vCellX + vCellY * subGridSize, true);
        }

        void clear() { visited.clear(); }
    };

    struct PathFindingData final
    {
        // Constants
        static constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
        static constexpr Point crossMove[4] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};

        // Config
        bool enableDynamic = true;

        // Grid data for each map - if cell is usable for pathfinding or not
        MapHolder<DenseLookupGrid<cellSize>> mapsStaticGrids;
        MapHolder<DenseLookupGrid<cellSize>> mapsDynamicGrids;

        // A star cache
        std::vector<Point> pathCache;
        DenseLookupGrid<cellSize> visited;
        cxstructs::PriorityQueue<GridNode> frontier{};
        GridNode nodePool[MAGIQUE_PATHFINDING_SEARCH_CAPACITY];

        //----------------- METHODS -----------------//

        // Checks if the given coordinates are in a solid tile - directly takes the grids to avoid the lookup
        [[nodiscard]] bool isCellSolid(const float x, const float y, const DenseLookupGrid<cellSize>& staticGrid,
                                       const DenseLookupGrid<cellSize>& dynamicGrid, const bool dynamic) const
        {
            if (staticGrid.getIsMarked(x, y)) [[unlikely]]
            {
                return true;
            }

            if (!dynamic) [[likely]]
                return false;

            return dynamicGrid.getIsMarked(x, y);
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
                auto& tileObjectInfo = staticData.colliderReferences.tileObjectMap.at(map);
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
                auto& objectIndices = staticData.colliderReferences.tilesCollisionMap.at(map);
                for (const auto idx : objectIndices)
                {
                    const auto& [x, y, w, h] = staticData.colliderStorage.get(idx);
                    rasterizeRect(x, y, w, h);
                }
            }
        }

        /*
        void updateDynamicGrid()
        {
            if (!enableDynamic)
                return;
            const auto& data = global::ENGINE_DATA;
            const auto group = internal::POSITION_GROUP;
            dynamicGrid.clear();
            const auto insertFunc = [this](const PathCellID id) { dynamicGrid[id] = true; };
            for (const auto e : data.collisionVec)
            {
                const auto& pos = group.get<const PositionC>(e);
                const auto& col = group.get<const CollisionC>(e);
                switch (col.shape) // Same as LogicSystem::HandleCollision
                {
                [[likely]] case Shape::RECT:
                    {
                        if (pos.rotation == 0) [[likely]]
                        {
                            RasterizeRect<cellSize>(insertFunc, GetPathCellID, pos.x, pos.y, col.p1, col.p2);
                            continue;
                        }
                        float pxs[4] = {0, col.p1, col.p1, 0};
                        float pys[4] = {0, 0, col.p2, col.p2};
                        RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                        const auto bb = GetBBQuadrilateral(pxs, pys);
                        return RasterizeRect<cellSize>(insertFunc, GetPathCellID, bb.x, bb.y, bb.width, bb.height);
                    }
                case Shape::CIRCLE:
                    return RasterizeRect<cellSize>(insertFunc, GetPathCellID, pos.x, pos.y, col.p1 * 2.0F,
                                                   col.p1 * 2.0F); // Top left and diameter as w and h
                case Shape::CAPSULE:
                    // Top left and height as height / diameter as w
                    return RasterizeRect<cellSize>(insertFunc, GetPathCellID, pos.x, pos.y, col.p1 * 2.0F, col.p2);
                case Shape::TRIANGLE:
                    {
                        if (pos.rotation == 0)
                        {
                            const auto bb = GetBBTriangle(pos.x, pos.y, pos.x + col.p1, pos.y + col.p2, pos.x + col.p3,
                                                          pos.y + col.p4);
                            RasterizeRect<cellSize>(insertFunc, GetPathCellID, bb.x, bb.y, bb.width, bb.height);
                            continue;
                        }
                        float txs[4] = {0, col.p1, col.p3, 0};
                        float tys[4] = {0, col.p2, col.p4, 0};
                        RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                        const auto bb = GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
                        RasterizeRect<cellSize>(insertFunc, GetPathCellID, bb.x, bb.y, bb.width, bb.height);
                    }
                }
            }
        }

*/

        void findPath(std::vector<Point>& path, const Point startC, const Point endC, const MapID map, const int maxLen,
                      const bool dynamic)
        {
            if (!mapsStaticGrids.contains(map) || !mapsDynamicGrids.contains(map))
                return;

            // Setup and get grids
            path.clear();
            frontier.clear();
            visited.clear();
            auto& staticGrid = mapsStaticGrids[map];
            auto& dynamicGrid = mapsDynamicGrids[map];

            const Point start = {std::floor(startC.x / cellSize), std::floor(startC.y / cellSize)};
            const Point end = {std::floor(endC.x / cellSize), std::floor(endC.y / cellSize)};

            // Viability check
            if (isCellSolid(end.x * cellSize, end.y * cellSize, staticGrid, dynamicGrid, dynamic))
            {
                LOG_WARNING("Cant search a path to a solid (or non existent) pathfinding cell!");
                return;
            }

            frontier.emplace(start, 0.0F, start.octile(end), UINT16_MAX);
            uint16_t counter = 0;

            while (!frontier.empty() && counter < maxLen)
            {
                nodePool[counter] = frontier.top();
                auto& current = nodePool[counter];
                if (current.position == end)
                {
                    return constructPath(current, path);
                }
                frontier.pop();
                visited.setMarked(current.position.x * cellSize, current.position.y * cellSize);
                for (const auto& dir : crossMove)
                {
                    Point newPos = {current.position.x + dir.x, current.position.y + dir.y};
                    const auto newPosCoX = newPos.x * cellSize;
                    const auto newPosCoY = newPos.y * cellSize;
                    // Is not visited and not solid
                    if (!visited.getIsMarked(newPosCoX, newPosCoY) &&
                        !isCellSolid(newPosCoX, newPosCoY, staticGrid, dynamicGrid, dynamic))
                    {
                        const float moveCost = dir.x != 0 && dir.y != 0 ? 1.4142F : 1.0F;
                        const auto hCost = newPos.octile(end) * 1.01F;
                        frontier.push({newPos, current.gCost + moveCost, hCost, counter});

                        // Debug
                        const Rectangle rect = {newPos.x * cellSize, newPos.y * cellSize, cellSize, cellSize};
                        DrawRectangleRec(rect, PURPLE);
                    }
                }
                counter++;
            }
        }

    private:
        void constructPath(const GridNode& current, std::vector<Point>& path) const
        {
            const GridNode* curr = &current;
            while (curr->parent != UINT16_MAX)
            {
                const Point p = {curr->position.x * cellSize + cellSize / 2.0F,
                                 curr->position.y * cellSize + cellSize / 2.0F};
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