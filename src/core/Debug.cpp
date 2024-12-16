// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Debug.h>
#include <magique/core/Types.h>
#include <magique/core/Camera.h>

#include "internal/globals/DynamicCollisionData.h"
#include "internal/globals/EngineConfig.h"
#include "internal/globals/PerformanceData.h"

namespace magique
{
    void SetShowPerformanceOverlay(const bool val, const bool extended)
    {
        global::ENGINE_CONFIG.showPerformanceOverlay = val;
        global::ENGINE_CONFIG.showPerformanceOverlayExt = extended;
    }

    void SetShowHitboxes(const bool val) { global::ENGINE_CONFIG.showHitboxes = val; }

    void SetShowEntityGridOverlay(const bool val) { global::ENGINE_CONFIG.showEntityOverlay = val; }

    void SetShowPathFindingOverlay(const bool val) { global::ENGINE_CONFIG.showPathFindingOverlay = val; }

    void SetShowCompassOverlay(const bool val) { global::ENGINE_CONFIG.showCompassOverlay = val; }

    void SetBenchmarkTicks(const int ticks) { global::ENGINE_CONFIG.benchmarkTicks = ticks; }

    void ResetPerformanceData()
    {
#if MAGIQUE_PROFILING == 1
        global::PERF_DATA.drawTimes.clear();
        global::PERF_DATA.logicTimes.clear();
#else
        LOG_WARNING("Calling BenchmarkFunction without profiling enabled. see config.h::MAGIQUE_PROFILING")
#endif
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