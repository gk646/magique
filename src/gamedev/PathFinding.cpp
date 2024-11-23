#include <raylib/raylib.h>

#include <magique/gamedev/PathFinding.h>
#include <magique/core/Core.h>

#include "internal/globals/PathFindingData.h"
#include "internal/headers/CollisionPrimitives.h"

namespace magique
{
    void FindPath(std::vector<Point>& pathVec, const Point start, const Point end, const MapID map, const int searchLen)
    {
        auto& path = global::PATH_DATA;
        constexpr int capacity = MAGIQUE_PATHFINDING_SEARCH_CAPACITY;
        const int maxLen = std::min(searchLen == 0 ? capacity : searchLen, capacity);
        path.findPath(pathVec, start, end, map, maxLen);
    }

    Point GetNextOnPath(const Point start, const Point end, const MapID map, const int searchLen)
    {
        auto& path = global::PATH_DATA;
        FindPath(path.pathCache, start, end, map, searchLen);
        if (path.pathCache.empty())
            return {0, 0};
        return path.pathCache[path.pathCache.size() - 1];
    }

    // Same as FindPath() but allows to specify the dimensions of the searching entity
    // The pathfinding tries to find a path that fits the entity
    void FindPathEx(std::vector<Point>& path, Point start, Point end, Point dimensions, int searchLen) {}

    bool GetPathRayCast(const Point start, const Point end, const MapID map)
    {
        auto& path = global::PATH_DATA; // Must exist - checked in CreateEntity()
        const auto& staticGrid = path.mapsStaticGrids[map];
        const auto& dynamicGrid = path.mapsDynamicGrids[map];

        int x0 = static_cast<int>(start.x / MAGIQUE_PATHFINDING_CELL_SIZE);
        int y0 = static_cast<int>(start.y / MAGIQUE_PATHFINDING_CELL_SIZE);
        int const x1 = static_cast<int>(end.x / MAGIQUE_PATHFINDING_CELL_SIZE);
        int const y1 = static_cast<int>(end.y / MAGIQUE_PATHFINDING_CELL_SIZE);

        int const dx = abs(x1 - x0);
        int const sx = x0 < x1 ? 1 : -1;
        int const dy = -abs(y1 - y0);
        int const sy = y0 < y1 ? 1 : -1;
        int error = dx + dy;

        while (true)
        {
            const float x = static_cast<float>(x0) * MAGIQUE_PATHFINDING_CELL_SIZE;
            const float y = static_cast<float>(y0) * MAGIQUE_PATHFINDING_CELL_SIZE;
            DrawRectangleRec({x, y, MAGIQUE_PATHFINDING_CELL_SIZE, MAGIQUE_PATHFINDING_CELL_SIZE}, PURPLE);
            if (PathFindingData::IsCellSolid(x, y, staticGrid, dynamicGrid))
            {
                return false;
            }
            if (x0 == x1 && y0 == y1)
                break;
            int const e2 = 2 * error;
            if (e2 >= dy)
            {
                error = error + dy;
                x0 = x0 + sx;
            }
            if (e2 <= dx)
            {
                error = error + dx;
                y0 = y0 + sy;
            }
        }
        return true;
    }

    void SetTypePathSolid(const EntityType type, const bool value)
    {
        if (value)
            global::PATH_DATA.solidTypes.insert(type);
        else
            global::PATH_DATA.solidTypes.erase(type);
    }

    bool GetIsTypePathSolid(const EntityType type) { return global::PATH_DATA.solidTypes.contains(type); }

    void SetEntityPathSolid(const entt::entity entity, const bool value)
    {
        if (value)
            global::PATH_DATA.solidEntities.insert(entity);
        else
            global::PATH_DATA.solidEntities.erase(entity);
    }

    bool GetIsEntityPathSolid(entt::entity entity) { return global::PATH_DATA.solidEntities.contains(entity); }

    Point GetDirectionVector(const Point current, const Point target)
    {
        const float diffX = target.x - current.x;
        const float diffY = target.y - current.y;
        float lenSquared = (diffX * diffX) + (diffY * diffY);
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
        if (!path.mapsDynamicGrids.contains(map)) // Could be called before any entity is created
            return;
        const auto& staticGrid = path.mapsStaticGrids[map];
        const auto& dynamicGrid = path.mapsDynamicGrids[map];

        const auto bounds = GetCameraBounds();
        constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
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
                const Rectangle rect = {(float)currX * cellSize, (float)currY * cellSize, cellSize, cellSize};
                const bool isSolid = PathFindingData::IsCellSolid(rect.x, rect.y, staticGrid, dynamicGrid);
                if (isSolid)
                {
                    DrawRectangleRec(rect, ColorAlpha(RED, 0.4F));
                }
                DrawRectangleLinesEx(rect, 1, BLACK);
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