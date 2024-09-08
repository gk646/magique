#include <magique/gamedev/Pathfinding.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Components.h>

#include "internal/globals/EngineData.h"
#include "internal/globals/StaticCollisionData.h"
#include "internal/globals/DynamicCollisionData.h"
#include "internal/datastructures/HashTypes.h"
#include "internal/headers/CollisionPrimitives.h"

namespace magique
{
    using PathCellID = int32_t;

    struct PathFindingData final
    {
        static constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
        HashMap<PathCellID, bool> staticGrid;
        HashMap<PathCellID, bool> dynamicGrid;

        bool isCellCSolid(const int cellX, const int cellY)
        {
            const auto id = GetCellID(cellX, cellY);
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
            const int normX = cellX / cellSize;
            const int normY = cellY / cellSize;

            constexpr int shiftAmount = sizeof(PathCellID) * 4;

            return static_cast<PathCellID>(normX) << shiftAmount | static_cast<PathCellID>(normY);
        }
    };

} // namespace magique