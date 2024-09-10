#ifndef MAGIQUE_PATHFINDING_DATA_H
#define MAGIQUE_PATHFINDING_DATA_H

#include <vector>

#include <magique/ecs/ECS.h>

#include "external/cxstructs/cxstructs/PriorityQueue.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/StaticCollisionData.h"
#include "internal/globals/DynamicCollisionData.h"
#include "internal/datastructures/HashTypes.h"
#include "internal/headers/CollisionPrimitives.h"

namespace magique
{
    using PathCellID = int32_t;

    struct GridNode final
    {
        Point position{};
        uint16_t fCost = 0;
        uint16_t gCost = 0;
        uint16_t parent = 0;
        GridNode() = default;
        GridNode(const Point position, uint16_t g_cost, uint16_t h_cost, uint16_t parent) :
            position(position), fCost(g_cost + h_cost), gCost(g_cost), parent(parent)
        {
        }
        bool operator<(const GridNode& o) const { return fCost < o.fCost; }
        bool operator>(const GridNode& o) const { return fCost > o.fCost; }
        bool operator==(const GridNode& o) const { return fCost == o.fCost; }
        bool operator<=(const GridNode& o) const { return fCost <= o.fCost; }
    };

    struct PathFindingData final
    {
        static constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
        static constexpr Point starMove[8] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

        HashMap<PathCellID, bool> staticGrid;
        HashMap<PathCellID, bool> dynamicGrid;
        std::vector<Point> pathCache;

        cxstructs::PriorityQueue<GridNode> frontier{};
        HashSet<PathCellID> visited{};
        GridNode nodePool[MAGIQUE_PATHFINDING_SEARCH_CAPACITY];

        bool isCellCSolid(const PathCellID id, const MapID map)
        {
            const auto staticIt = staticGrid.find(id);
            if (staticIt != staticGrid.end()) [[unlikely]]
            {
                if (staticIt->second)
                {
                    return true;
                }
            }

            const auto dynamicIt = dynamicGrid.find(id);
            if (dynamicIt != dynamicGrid.end())
            {
                return dynamicIt->second;
            }
            return false;
        }

        void updateStaticGrid()
        {
            const auto& staticData = global::STATIC_COLL_DATA;
            staticGrid.clear();
            const auto insertFunc = [this](const PathCellID id) { staticGrid[id] = true; };
            for (const auto& obj : staticData.objectStorage.colliders)
            {
                RasterizeRect<cellSize>(insertFunc, GetCellID, obj.x, obj.y, obj.p1, obj.p2);
            }
        }

        void updateDynamicGrid()
        {
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
                            return RasterizeRect<cellSize>(insertFunc, GetCellID, pos.x, pos.y, col.p1, col.p2);
                        }
                        float pxs[4] = {0, col.p1, col.p1, 0};
                        float pys[4] = {0, 0, col.p2, col.p2};
                        RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                        const auto bb = GetBBQuadrilateral(pxs, pys);
                        return RasterizeRect<cellSize>(insertFunc, GetCellID, bb.x, bb.y, bb.width, bb.height);
                    }
                case Shape::CIRCLE:
                    return RasterizeRect<cellSize>(insertFunc, GetCellID, pos.x, pos.y, col.p1 * 2.0F,
                                                   col.p1 * 2.0F); // Top left and diameter as w and h
                case Shape::CAPSULE:
                    // Top left and height as height / diameter as w
                    return RasterizeRect<cellSize>(insertFunc, GetCellID, pos.x, pos.y, col.p1 * 2.0F, col.p2);
                case Shape::TRIANGLE:
                    {
                        if (pos.rotation == 0)
                        {
                            const auto bb = GetBBTriangle(pos.x, pos.y, pos.x + col.p1, pos.y + col.p2, pos.x + col.p3,
                                                          pos.y + col.p4);
                            return RasterizeRect<cellSize>(insertFunc, GetCellID, bb.x, bb.y, bb.width, bb.height);
                        }
                        float txs[4] = {0, col.p1, col.p3, 0};
                        float tys[4] = {0, col.p2, col.p4, 0};
                        RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                        const auto bb = GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
                        RasterizeRect<cellSize>(insertFunc, GetCellID, bb.x, bb.y, bb.width, bb.height);
                    }
                }
            }
        }


        static [[nodiscard]] PathCellID GetCellID(const int cellX, const int cellY)
        {
            constexpr int shiftAmount = sizeof(PathCellID) * 4;
            return static_cast<PathCellID>(cellX) << shiftAmount | static_cast<PathCellID>(cellY);
        }

        void findPath(std::vector<Point>& path, const Point startC, const Point endC, const MapID map, const int maxLen)
        {
            path.clear();
            frontier.clear();
            visited.clear();

            const Point start = {startC.x / cellSize, startC.y / cellSize};
            const Point end = {endC.x / cellSize, endC.y / cellSize};

            frontier.emplace(start, static_cast<uint16_t>(0), static_cast<uint16_t>(start.manhattan(end)), UINT16_MAX);
            uint16_t counter = 0;

            while (!frontier.empty() && counter < maxLen && counter < MAGIQUE_PATHFINDING_SEARCH_CAPACITY)
            {
                nodePool[counter] = frontier.top();
                auto& current = nodePool[counter];
                if (current.position == end)
                {
                    return constructPath(current, path);
                }
                frontier.pop();
                visited.insert(GetCellID(static_cast<int>(current.position.x), static_cast<int>(current.position.y)));
                for (const auto dir : starMove)
                {
                    Point newPos = {current.position.x + dir.x, current.position.y + dir.y};
                    const auto id = GetCellID(static_cast<int>(newPos.x), static_cast<int>(newPos.y));
                    if (!visited.contains(id) && !isCellCSolid(id, map))
                    {
                        const auto hCost = static_cast<uint16_t>(std::round(newPos.manhattan(end)));
                        frontier.push({newPos, static_cast<uint16_t>(current.gCost + 1), hCost, counter});
                    }
                }
                counter++;
            }
        }

    private:
        void constructPath(GridNode current, std::vector<Point>& path) {}
    };

    namespace global
    {
        inline PathFindingData PATH_DATA{};
    }
} // namespace magique

#endif //MAGIQUE_PATHFINDING_DATA_H