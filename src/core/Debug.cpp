#include <magique/core/Debug.h>
#include <magique/core/Types.h>

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
        if (!dynamic.mapEntityGrids.contains(map))
            return;
        const auto& grid = dynamic.mapEntityGrids[map];
        int half = MAGIQUE_COLLISION_CELL_SIZE / 2;
        for (int i = 0; i < 50; ++i)
        {
            for (int j = 0; j < 50; ++j)
            {
                int x = i * MAGIQUE_COLLISION_CELL_SIZE;
                int y = j * MAGIQUE_COLLISION_CELL_SIZE;

                DrawRectangleLines(x, y, MAGIQUE_COLLISION_CELL_SIZE, MAGIQUE_COLLISION_CELL_SIZE, LIGHTGRAY);
                auto id = GetCellID(x / grid.getCellSize(), y / grid.getCellSize());
                const auto it = grid.cellMap.find(id);
                if (it != grid.cellMap.end())
                {
                    const auto count = static_cast<int>(grid.dataBlocks[it->second].count);
                    const auto color = count > grid.getBlockSize() ? RED : GREEN;
                    DrawText(std::to_string(count).c_str(), x + half, y + half, 20, color);
                }
            }
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