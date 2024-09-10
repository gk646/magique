#include <magique/gamedev/Pathfinding.h>

#include "internal/globals/PathFindingData.h"

namespace magique
{
    void GetShortestPath(std::vector<Point>& pathVec, const Point start, const Point end, const MapID map,
                         const int searchLen)
    {
        auto& path = global::PATH_DATA;
        path.findPath(pathVec, start, end, map, searchLen);
    }

    Point GetNextPosition(const Point start, const Point end, const MapID map, const int searchLen)
    {
        auto& path = global::PATH_DATA;
        path.findPath(path.pathCache, start, end, map, searchLen);
        return path.pathCache[0];
    }
} // namespace magique