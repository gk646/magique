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
// For collision lookups hashmaps are used -> can be optimized a lot
//
// .....................................................................

namespace magique
{
    using PathCellID = int32_t;

    static [[nodiscard]] PathCellID GetPathCellID(const int cellX, const int cellY)
    {
        constexpr auto size = sizeof(PathCellID);
        return static_cast<PathCellID>(cellX) << size * 4 | static_cast<PathCellID>(cellY);
    }

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

    struct DenseLookupGrid final
    {
        using VisitedCellID = int32_t;

        static [[nodiscard]] VisitedCellID GetVisitedCell(const int cellX, const int cellY)
        {
            constexpr auto size = sizeof(VisitedCellID);
            return static_cast<VisitedCellID>(cellX) << size * 4 | static_cast<VisitedCellID>(cellY);
        }

        constexpr static int subGridSize = 16; // Fits into cache line (key/value pair)
        constexpr static int mainGridSize = MAGIQUE_PATHFINDING_CELL_SIZE * subGridSize;

        HashMap<PathCellID, std::bitset<subGridSize * subGridSize>> visited{};

        [[nodiscard]] bool isVisited(const float x, const float y) const
        {
            const int cellX = static_cast<int>(x) / mainGridSize;
            const int cellY = static_cast<int>(y) / mainGridSize;
            const auto it = visited.find(GetPathCellID(cellX, cellY));
            if (it != visited.end())
            {
                const int vCellX = (static_cast<int>(x) - cellX * mainGridSize) / MAGIQUE_PATHFINDING_CELL_SIZE;
                const int vCellY = (static_cast<int>(y) - cellY * mainGridSize) / MAGIQUE_PATHFINDING_CELL_SIZE;
                return it->second[vCellX + vCellY * subGridSize];
            }
            return false;
        }

        void markVisited(const float x, const float y)
        {
            const int cellX = static_cast<int>(x) / mainGridSize;
            const int cellY = static_cast<int>(y) / mainGridSize;
            const int vCellX = (static_cast<int>(x) - cellX * mainGridSize) / MAGIQUE_PATHFINDING_CELL_SIZE;
            const int vCellY = (static_cast<int>(y) - cellY * mainGridSize) / MAGIQUE_PATHFINDING_CELL_SIZE;
            visited[GetVisitedCell(cellX, cellY)].set(vCellX + vCellY * subGridSize, true);
        }

        void clear() { visited.clear(); }
    };

    struct PathFindingData final
    {
        // Constants
        static constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
        static constexpr Point starMove[8] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
        static constexpr Point crossMove[4] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};

        // Config
        bool enableDynamic = true;

        DenseLookupGrid staticGrid;
        DenseLookupGrid dynamicGrid;
        std::vector<Point> pathCache;

        DenseLookupGrid visited;
        cxstructs::PriorityQueue<GridNode> frontier{};
        GridNode nodePool[MAGIQUE_PATHFINDING_SEARCH_CAPACITY];

        [[nodiscard]] bool isCellSolid(const float x, const float y,  const MapID map, const bool dynamic) const
        {
            const auto staticIt = staticGrid.find(id);
            if (staticGrid.isVisited()) [[unlikely]]
            {
                return true;
            }

            if (!dynamic)
                return false;

            const auto dynamicIt = dynamicGrid.find(id);
            return dynamicIt != dynamicGrid.end() && dynamicIt->second;
        }

        void updateStaticGrid()
        {
            const auto& staticData = global::STATIC_COLL_DATA;
            staticGrid.clear();
            for (const auto& obj : staticData.objectStorage.colliders)
            {
                const int startX = static_cast<int>(obj.x) / cellSize;
                const int startY = static_cast<int>(obj.y) / cellSize;
                const int endX = static_cast<int>(obj.x + obj.p1) / cellSize;
                const int endY = static_cast<int>(obj.y + obj.p2) / cellSize;

                // Loop through potentially intersecting grid cells
                for (int i = startY; i <= endY; ++i)
                {
                    const auto cellY = static_cast<float>(i) * cellSize;
                    for (int j = startX; j <= endX; ++j)
                    {
                        const auto cellX = static_cast<float>(j) * cellSize;
                        if (RectToRect(obj.x, obj.y, obj.p1, obj.p2, cellX, cellY, cellSize, cellSize))
                        {
                            staticGrid[GetPathCellID(j, i)] = true;
                        }
                    }
                }
            }
        }

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

        void findPath(std::vector<Point>& path, const Point startC, const Point endC, const MapID map, const int maxLen,
                      const bool dynamic)
        {
            path.clear();
            frontier.clear();
            visited.clear();

            const Point start = {std::floor(startC.x / cellSize), std::floor(startC.y / cellSize)};
            const Point end = {std::floor(endC.x / cellSize), std::floor(endC.y / cellSize)};

            if (isCellSolid(GetPathCellID(static_cast<int>(end.x), static_cast<int>(end.y)), map, dynamic))
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
                visited.markVisited(current.position.x * cellSize, current.position.y * cellSize);
                for (const auto& dir : crossMove)
                {
                    Point newPos = {current.position.x + dir.x, current.position.y + dir.y};
                    const auto newPosCoX = newPos.x * cellSize;
                    const auto newPosCoY = newPos.y * cellSize;
                    const auto id = GetPathCellID(static_cast<int>(newPos.x), static_cast<int>(newPos.y));
                    if (!visited.isVisited(newPosCoX, newPosCoY) && !isCellSolid(id, map, dynamic))
                    {
                        const float moveCost = dir.x != 0 && dir.y != 0 ? 1.4142F : 1.0F;
                        const auto hCost = newPos.octile(end) * 1.01F;
                        frontier.push({newPos, current.gCost + moveCost, hCost, counter});
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