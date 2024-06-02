#ifndef HASHGRID_H
#define HASHGRID_H

#include "external/raylib/src/raylib.h"


#include <ankerl/unordered_dense.h>
#include "fastvector/fast_vector.h"

// Old hash grid
// Naive implementation

/**
 * HashGrid for a square space
 */
template <typename EntityID = uint32_t>
struct HashGrid
{
    using size_type = uint32_t;
    using GridID = size_type;
    using value_type = fast_vector<EntityID>;

    ankerl::unordered_dense::map<GridID, value_type> map;

private:
    float cellSize = 0;
    float spaceSize = 0;
    size_type gridSize = 0;

public:
    explicit HashGrid(float cellSize, float spaceSize) :
        cellSize(cellSize), spaceSize(spaceSize), gridSize(static_cast<size_type>(spaceSize / cellSize))
    {
        map.reserve(gridSize * gridSize);
    };
    HashGrid() = default;
    HashGrid(const HashGrid&) = delete;
    HashGrid& operator=(const HashGrid&) = delete;
    HashGrid(HashGrid&&) = delete;
    HashGrid& operator=(HashGrid&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        map = std::move(other.map);
        cellSize = other.cellSize;
        spaceSize = other.spaceSize;
        gridSize = other.gridSize;

        other.cellSize = 0;
        other.spaceSize = 0;
        other.gridSize = 0;

        return *this;
    }
    inline const value_type& operator[](GridID g) noexcept { return map[g]; }
    [[nodiscard]] inline GridID getGridID(float x, float y) const noexcept
    {
        return static_cast<int>(x / cellSize) + static_cast<int>(y / cellSize) * gridSize;
    }
    inline void clear()
    {
        for (auto begin = map.begin(); begin != map.end(); ++begin)
        {
            begin.value().clear();
        }
    }
    inline void setupNew(float newCellSize, uint16_t newSpaceSize, bool optimized = true)
    {
        if (optimized)
        {
            float value = cellSize / spaceSize;
            cellSize = newSpaceSize * value;
            map.reserve(std::pow(spaceSize / cellSize, 2));
        }
        else
        {
            cellSize = newCellSize;
            spaceSize = newSpaceSize;
        }
        map.clear();
    };
    inline void getGridIDs(std::array<int16_t, 4>& gridIDs, float x, float y, float width, float height) const noexcept
    {
        const auto topLeftGridX = static_cast<int>(x / cellSize);
        const auto topLeftGridY = static_cast<int>(y / cellSize);
        const auto bottomRightGridX = static_cast<int>((x + width) / cellSize);
        const auto bottomRightGridY = static_cast<int>((y + height) / cellSize);

        if (topLeftGridX == bottomRightGridX && topLeftGridY == bottomRightGridY)
        {
            gridIDs[0] = topLeftGridX + topLeftGridY * gridSize;
            return;
        }

        const auto topLeftIndex = topLeftGridX + topLeftGridY * gridSize;
        const auto bottomRightIndex = bottomRightGridX + bottomRightGridY * gridSize;

        gridIDs[0] = topLeftIndex;
        gridIDs[1] = (topLeftGridX != bottomRightGridX) ? topLeftIndex + 1 : -1;
        gridIDs[2] = (topLeftGridY != bottomRightGridY) ? topLeftIndex + gridSize : -1;
        gridIDs[3] = (gridIDs[1] != -1 && gridIDs[2] != -1) ? bottomRightIndex : -1;
    }

    inline void insert(float x, float y, float width, float height, float rot, float rotX, float rotY,
                       EntityID entityID)
    {
        if (rot == 0.0F || (rotX == 0 && rotY == 0))
        {
            const int topLeftGridX = static_cast<int>(x / cellSize);
            const int topLeftGridY = static_cast<int>(y / cellSize);
            const int bottomRightGridX = static_cast<int>((x + width) / cellSize);
            const int bottomRightGridY = static_cast<int>((y + height) / cellSize);

            if (topLeftGridX == bottomRightGridX && topLeftGridY == bottomRightGridY)
            {
                int gridID = topLeftGridX + topLeftGridY * gridSize;
                map[gridID].push_back(entityID);
                return;
            }

            int topLeftIndex = topLeftGridX + topLeftGridY * gridSize;
            map[topLeftIndex].push_back(entityID);

            if (topLeftGridX != bottomRightGridX)
            {
                int topRightIndex = topLeftIndex + 1;
                map[topRightIndex].push_back(entityID);
            }

            if (topLeftGridY != bottomRightGridY)
            {
                int bottomLeftIndex = topLeftIndex + gridSize;
                map[bottomLeftIndex].push_back(entityID);
            }

            if (topLeftGridX != bottomRightGridX && topLeftGridY != bottomRightGridY)
            {
                int bottomRightIndex = bottomRightGridX + bottomRightGridY * gridSize;
                map[bottomRightIndex].push_back(entityID);
            }
        }
        else
        {
            float rad = rot * DEG2RAD;
            float cosTheta = cosf(rad);
            float sinTheta = sinf(rad);

            // Define local space rectangle corners
            Vector2 localCorners[4] = {
                {-rotX, -rotY},                // top-left
                {width - rotX, -rotY},         // top-right
                {width - rotX, height - rotY}, // bottom-right
                {-rotX, height - rotY}         // bottom-left
            };

            // Initialize an array to hold the grid IDs for each corner
            std::array<int16_t, 4> cornerGridIDs = {-1, -1, -1, -1};

            // Rotate each corner around the local pivot and translate to world space
            // Then calculate the grid ID for each rotated corner
            for (int i = 0; i < 4; ++i)
            {
                float localX = localCorners[i].x;
                float localY = localCorners[i].y;

                // Apply rotation
                float rotatedX = localX * cosTheta - localY * sinTheta;
                float rotatedY = localX * sinTheta + localY * cosTheta;

                // Translate back to world position
                float worldX = x + rotatedX + rotX;
                float worldY = y + rotatedY + rotY;

                // Calculate grid ID for this corner
                int gridX = static_cast<int>(worldX / cellSize);
                int gridY = static_cast<int>(worldY / cellSize);
                cornerGridIDs[i] = gridX + gridY * gridSize;
            }

            for (int16_t gridID : cornerGridIDs)
            {
                if (gridID != -1)
                {
                    map[gridID].push_back(entityID);
                }
            }
        }
    }
};

#endif //HASHGRID_H