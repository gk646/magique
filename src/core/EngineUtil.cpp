// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/EngineUtil.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/PerformanceData.h"

namespace magique
{
    void EngineShowPerformanceOverlay(const bool val, const bool extended)
    {
        global::ENGINE_CONFIG.showPerformanceOverlay = val;
        global::ENGINE_CONFIG.showPerformanceOverlayExt = extended;
    }

    void EngineShowHitboxes(const bool val) { global::ENGINE_CONFIG.showHitboxes = val; }

    void EngineShowEntityOverlay(const bool val) { global::ENGINE_CONFIG.showEntityOverlay = val; }

    void EngineShowPathfindingOverlay(const bool val) { global::ENGINE_CONFIG.showPathFindingOverlay = val; }

    void EngineShowCompass(const bool val) { global::ENGINE_CONFIG.showCompassOverlay = val; }

    void EngineSetBenchmarkTicks(const int ticks) { global::ENGINE_CONFIG.benchmarkTicks = ticks; }

    void EngineResetBenchmark()
    {
        global::PERF_DATA.drawTick.reset();
        global::PERF_DATA.updateTick.reset();
    }
} // namespace magique