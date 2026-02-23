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
        float fCost; // Combined cost
        float gCost; // Cost from start
        uint16_t parent;
        uint16_t stepCount;
        GridNode() = default;
        GridNode(const Point position, const float gCost, const float fCost, const uint16_t parent,
                 const uint16_t stepCount) :
            position(position), fCost(fCost), gCost(gCost), parent(parent), stepCount(stepCount)
        {
        }
        bool operator>(const GridNode& o) const { return fCost > o.fCost; }
        bool operator==(const GridNode& o) const { return position == o.position; }
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
            const int cellX = floordiv<mainGridSize>(x);
            const int cellY = floordiv<mainGridSize>(y);
            const auto pathCell = GetVisitedCell(cellX, cellY);
            const auto it = visited.find(pathCell);
            if (it != visited.end()) // Get the position within the subgrid
            {
                const int vCellX = std::abs(static_cast<int>(x)) % mainGridSize / mainGridBaseSize;
                const int vCellY = std::abs(static_cast<int>(y)) % mainGridSize / mainGridBaseSize;
                return it->second[vCellX + (vCellY * subGridSize)];
            }
            return false;
        }

        void setMarked(const float x, const float y)
        {
            const int cellX = floordiv<mainGridSize>(x);
            const int cellY = floordiv<mainGridSize>(y);
            const int vCellX = std::abs(static_cast<int>(x)) % mainGridSize / mainGridBaseSize;
            const int vCellY = std::abs(static_cast<int>(y)) % mainGridSize / mainGridBaseSize;
            const auto cell = GetVisitedCell(cellX, cellY);
            visited[cell].set(vCellX + (vCellY * subGridSize), true);
        }

        void insert(const float x, const float y, const float w, const float h)
        {
            auto insertFunc = [this](const int cellX, const int cellY)
            {
                setMarked((cellX * MAGIQUE_PATHFINDING_CELL_SIZE), (cellY * MAGIQUE_PATHFINDING_CELL_SIZE));
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

        void clear() { memset(rows, 0, size * size * sizeof(T)); }
    };


} // namespace magique
#endif // PATHFINDINGSTRUCTS_H
