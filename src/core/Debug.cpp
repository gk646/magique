// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Debug.h>
#include <magique/core/Types.h>
#include <magique/core/Core.h>

#include "internal/globals/DynamicCollisionData.h"
#include "internal/globals/EngineConfig.h"
#include "internal/globals/PerformanceData.h"

namespace magique
{
    void SetShowHitboxes(const bool val) { global::ENGINE_CONFIG.showHitboxes = val; }

    void SetBenchmarkTicks(const int ticks) { global::ENGINE_CONFIG.benchmarkTicks = ticks; }

    void ResetBenchmarkTimes()
    {
#if MAGIQUE_PROFILING == 1
        global::PERF_DATA.drawTimes.clear();
        global::PERF_DATA.logicTimes.clear();
#endif
    }

    void DrawHashGridDebug(const MapID map)
    {
        const auto& dynamic = global::DY_COLL_DATA;
        if (!dynamic.mapEntityGrids.contains(map)) // Could be called before any entity is created
            return;
        const auto& grid = dynamic.mapEntityGrids[map];

        const auto bounds = GetCameraBounds();
        constexpr int cellSize = MAGIQUE_COLLISION_CELL_SIZE;
        const int startX = static_cast<int>(bounds.x) / cellSize;
        const int startY = static_cast<int>(bounds.y) / cellSize;
        const int width = static_cast<int>(bounds.width) / cellSize;
        const int height = static_cast<int>(bounds.height) / cellSize;

        const int half = MAGIQUE_COLLISION_CELL_SIZE / 2;
        for (int i = 0; i < height; ++i)
        {
            const int currY = startY + i;
            for (int j = 0; j < width; ++j)
            {
                const int currX = startX + j;
                const int x = currX * MAGIQUE_COLLISION_CELL_SIZE;
                const int y = currY * MAGIQUE_COLLISION_CELL_SIZE;

                const auto id = GetCellID(currX, currY);
                const auto it = grid.cellMap.find(id);
                if (it != grid.cellMap.end())
                {
                    const auto count = static_cast<int>(grid.dataBlocks[it->second].count);
                    const auto color = count > grid.getBlockSize() ? RED : GREEN; // Over the limit
                    DrawText(std::to_string(count).c_str(), x + half, y + half, 20, color);
                }
                DrawRectangleLines(x, y, MAGIQUE_COLLISION_CELL_SIZE, MAGIQUE_COLLISION_CELL_SIZE, BLACK);
            }
        }
    }

    void Draw2DCompass(const Color& color)
    {
        constexpr float DISTANCE = 10'000;
        constexpr int MARKER_GAP = 500; // Pixels between each marker
        constexpr int MARKER_SIZE = 30; // Pixels between each marker

        const auto bounds = GetCameraBounds();
        const int startX = static_cast<int>(bounds.x) / MARKER_GAP;
        const int startY = static_cast<int>(bounds.y) / MARKER_GAP;
        const int width = static_cast<int>(bounds.width) / MARKER_GAP;
        const int height = static_cast<int>(bounds.height) / MARKER_GAP;

        DrawLineEx({-DISTANCE, 0}, {DISTANCE, 0},2, color);
        DrawLineEx({0, -DISTANCE}, {0, DISTANCE},2, color);

        for (int i = 0; i < width; ++i)
        {
            const int currX = startX + i;
            const int x = currX * MARKER_GAP;
            DrawLine(x, -MARKER_SIZE, x, MARKER_SIZE, color);
        }
        for (int i = 0; i < height; ++i)
        {
            const int currY = startY + i;
            const int y = currY * MARKER_GAP;
            DrawLine(-MARKER_SIZE, y, MARKER_SIZE, y, color);
        }
    }

    inline static uint64_t START_TIMERS[MAGIQUE_MAX_SUPPORTED_TIMERS]{};
    inline static int id[MAGIQUE_MAX_SUPPORTED_TIMERS]{};

    void StartTimer(const int num)
    {
        for (int i = 0; i < MAGIQUE_MAX_SUPPORTED_TIMERS; ++i)
        {
            if (id[i] == 0)
            {
                id[i] = num;
                START_TIMERS[i] = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                return;
            }
        }
        LOG_WARNING("No available timer slots!");
    }

    int GetTimerTime(const int num)
    {
        for (int i = 0; i < MAGIQUE_MAX_SUPPORTED_TIMERS; ++i)
        {
            if (id[i] == num)
            {
                const uint64_t endTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                const int elapsedTime = static_cast<int>(endTime - START_TIMERS[i]);
                return elapsedTime;
            }
        }
        LOG_WARNING("Timer with ID %d not found!", num);
        return -1;
    }

    int StopTimer(const int num)
    {
        for (int i = 0; i < MAGIQUE_MAX_SUPPORTED_TIMERS; ++i)
        {
            if (id[i] == num)
            {
                const uint64_t endTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                const int elapsedTime = static_cast<int>(endTime - START_TIMERS[i]);
                id[i] = 0;
                return elapsedTime;
            }
        }
        LOG_WARNING("Timer with ID %d not found!", num);
        return -1;
    }

} // namespace magique