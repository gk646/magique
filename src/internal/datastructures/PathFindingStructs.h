#ifndef PATHFINDINGSTRUCTS_H
#define PATHFINDINGSTRUCTS_H

#include <bitset>
#include "magique/util/Datastructures.h"

namespace magique
{
    inline constexpr std::array<PathFindHeuristicFunc, 2> PATH_HEURISTICS = {
        [](const Point& curr, const Point& end) { return curr.manhattan(end) * 1.1F; },
        [](const Point& curr, const Point& end) { return curr.chebyshev(end) * 1.1F; },
    };
    inline constexpr std::array<PathFindMoveCostFunc, 2> MOVE_COST = {
        [](const Point& dir) { return 1.0F; },
        [](const Point& dir) { return dir.x != 0 && dir.y != 0 ? 1.40F : 1.0F; },
    };

    inline std::array MOVEMENTS = {StackVector<Point, 8>{
                                       Point{0, -1}, // North
                                       {1, 0},       // East
                                       {0, 1},       // South
                                       {-1, 0},      // West
                                   },
                                   StackVector<Point, 8>{
                                       Point{0, -1}, // North
                                       {1, -1},      // North-East
                                       {1, 0},       // East
                                       {1, 1},       // South-East
                                       {0, 1},       // South
                                       {-1, 1},      // South-West
                                       {-1, 0},      // West
                                       {-1, -1}      // North-West
                                   }};

    struct GridNode final
    {
        Point position;
        float fCost; // Combined cost of moving + cost of distance
        float gCost; // Cost of moving
        float hCost; // Cost of distance
        uint16_t parent;
        uint16_t stepCount;
        GridNode() = default;
        GridNode(Point position, float gCost, float fCost, float hCost, uint16_t parent, uint16_t stepCount) :
            position(position), fCost(fCost), gCost(gCost), hCost(hCost), parent(parent), stepCount(stepCount)
        {
        }
        bool operator>(const GridNode& o) const { return fCost > o.fCost; }
        bool operator==(const GridNode& o) const { return position == o.position; }
    };

    using VisitedCellID = uint32_t;

    static VisitedCellID GetVisitedCellID(const int cellX, const int cellY)
    {
        // VisitedCellID must be uint otherwise the shifting doesn't work
        // A signed int has its sign saved in the most significant bit regardless of size
        // Thus shifting the number left shifts it away...
        // The casting of negative numbers to uint results in: Largest value - abs(value)
        // So negative numbers go down from the top - which is good for hash distribution anyway
        const auto first = static_cast<uint16_t>(cellX);
        const auto second = static_cast<uint16_t>(cellY);
        return (static_cast<VisitedCellID>(first) << 16) | second;
    }

    template <int mainGridBaseSize>
    struct DenseLookupGrid final
    {
        HashMap<VisitedCellID, bool> visited{};

        [[nodiscard]] bool getIsMarked(const float x, const float y) const
        {
            const int cellX = floordiv<mainGridBaseSize>(x);
            const int cellY = floordiv<mainGridBaseSize>(y);
            return visited.contains(GetVisitedCellID(cellX, cellY));
        }

        void setMarked(const float x, const float y)
        {
            const int cellX = floordiv<mainGridBaseSize>(x);
            const int cellY = floordiv<mainGridBaseSize>(y);
            visited[GetVisitedCellID(cellX, cellY)] = true;
        }

        void insert(const float x, const float y, const float w, const float h)
        {
            auto insertFunc = [this](const int cellX, const int cellY)
            {
                setMarked(static_cast<float>(cellX * MAGIQUE_PATHFINDING_CELL_SIZE),
                          static_cast<float>(cellY * MAGIQUE_PATHFINDING_CELL_SIZE));
            };
            RasterizeRect<mainGridBaseSize>(insertFunc, x, y, w, h);
        }

        void clear() { visited.clear(); }
    };

    // Lookup grid that takes all given positions relative to its center
    // This works good inside a single search - cleared between each search
    // Allows very fast lookups without a hashmap
    template <typename T, int size>
    struct StaticDenseLookupGrid final
    {
        std::array<T, size> rows[size]{};
        int midX;
        int midY;

        void setNewMid(const Point& mid)
        {
            midX = static_cast<int>(mid.x);
            midY = static_cast<int>(mid.y);
            clear();
        }

        [[nodiscard]] T getValue(const float x, const float y) const
        {
            const auto relX = static_cast<int>(x) - midX + size / 2;
            const auto relY = static_cast<int>(y) - midY + size / 2;
            if (relX >= 0 && relX < size && relY >= 0 && relY < size) [[likely]]
            {
                return rows[relY][relX];
            }
            return 0;
        }

        [[nodiscard]] T getValue(const Point& p) const { return getValue(p.x, p.y); }

        void setValue(const float x, const float y, const T& value)
        {
            const auto relX = static_cast<int>(x) - midX + size / 2;
            const auto relY = static_cast<int>(y) - midY + size / 2;
            if (relX >= 0 && relX < size && relY >= 0 && relY < size) [[likely]]
            {
                rows[relY][relX] = value;
            }
        }

        void setValue(const Point& p, const T& value) { setValue(p.x, p.y, value); }

        void clear() { std::memset(rows, 0, size * size * sizeof(T)); }
    };


} // namespace magique
#endif // PATHFINDINGSTRUCTS_H
