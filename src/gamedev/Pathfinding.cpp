#include <magique/gamedev/Pathfinding.h>
#include <magique/core/Core.h>

#include "internal/globals/PathFindingData.h"
#include "internal/headers/CollisionPrimitives.h"

namespace magique
{
    void FindPath(std::vector<Point>& pathVec, const Point start, const Point end, const MapID map, const int searchLen,
                  const bool dynamic)
    {
        auto& path = global::PATH_DATA;
        constexpr int capacity = MAGIQUE_PATHFINDING_SEARCH_CAPACITY;
        const int maxLen = std::min(searchLen == 0 ? capacity : searchLen, capacity);
        path.findPath(pathVec, start, end, map, maxLen, dynamic);
    }

    Point GetNextOnPath(const Point start, const Point end, const MapID map, const int searchLen, const bool dynamic)
    {
        auto& path = global::PATH_DATA;
        FindPath(path.pathCache, start, end, map, searchLen, dynamic);
        if (path.pathCache.empty())
            return {0, 0};
        return path.pathCache[path.pathCache.size() - 1];
    }

    Point GetDirectionVector(const Point current, const Point target)
    {
        const float diffX = target.x - current.x;
        const float diffY = target.y - current.y;
        float lenSquared = diffX * diffX + diffY * diffY;
        if (lenSquared != 0.0f)
        {
            SquareRoot(lenSquared);
            const float invLen = 1.0f / lenSquared;
            return {diffX * invLen, diffY * invLen};
        }
        return {0.0f, 0.0f};
    }

    void DrawPathFindingGrid(const MapID map)
    {
        const auto& path = global::PATH_DATA;
        const auto bounds = GetCameraBounds();
        constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
        constexpr float cellSizeF = MAGIQUE_PATHFINDING_CELL_SIZE;
        const int startX = static_cast<int>(bounds.x) / cellSize;
        const int startY = static_cast<int>(bounds.y) / cellSize;
        const int width = static_cast<int>(bounds.width) / cellSize;
        const int height = static_cast<int>(bounds.height) / cellSize;
        for (int i = 0; i < height; ++i)
        {
            const int currY = startY + i;
            for (int j = 0; j < width; ++j)
            {
                const int currX = startX + j;
                const auto id = GetPathCellID(currX, currY);
                const bool isSolid = path.isCellSolid(id, map, true);
                const Rectangle rect = {(float)currX * cellSizeF, (float)currY * cellSizeF, cellSizeF, cellSizeF};
                DrawRectangleLinesEx(rect, 1, ColorAlpha(GRAY, 0.4F));
                if (!isSolid)
                {
                    DrawRectangleRec(rect, ColorAlpha(GREEN, 0.4F));
                }
            }
        }
    }

    void DrawPath(const std::vector<Point>& path)
    {
        constexpr int halfSize = MAGIQUE_PATHFINDING_CELL_SIZE / 2;
        constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
        for (const auto p : path)
        {
            const Rectangle rect = {p.x - halfSize, p.y - halfSize, cellSize, cellSize};
            DrawRectangleRec(rect, RED);
        }
    }

} // namespace magique