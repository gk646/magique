#ifndef PERFDATA_H
#define PERFDATA_H

#include <numeric>
#include <vector>

#include "ui/internal/PerformanceDisplay.h"
#include "core/Config.h"

namespace magique
{
    enum TickType
    {
        UPDATE,
        DRAW
    };

    struct PerfData
    {
        uint32_t logicTickTime = 0;
        uint32_t drawTickTime = 0;
        PerformanceDisplay perfOverlay;
#ifdef MAGIQUE_DEBUG_PROFILE
        std::vector<uint32_t> logicTimes;
        std::vector<uint32_t> drawTimes;
#endif

        PerfData()
        {
#ifdef MAGIQUE_DEBUG_PROFILE
            // Reserve much upfront to not impede benchmarks
            logicTimes.reserve(10000);
            drawTimes.reserve(10000);
#endif
        }

        void saveTickTime(const TickType t, const uint32_t time)
        {
            if (t == UPDATE)
            {
                logicTickTime = time;
#ifdef MAGIQUE_DEBUG_PROFILE
                logicTimes.push_back(time);
#endif
            }
            else if (t == DRAW)
            {
                drawTickTime = time;
#ifdef MAGIQUE_DEBUG_PROFILE
                drawTimes.push_back(time);
#endif
            }
        }

        // Dont even let it be there
#ifdef MAGIQUE_DEBUG_PROFILE
        [[nodiscard]] float getAverageTime(const TickType t) const
        {
            const std::vector<uint32_t>* times;
            if (t == UPDATE)
            {
                times = &logicTimes;
            }
            else if (t == DRAW)
            {
                times = &drawTimes;
            }
            else
            {
                return 0.0f;
            }

            if (times->empty())
            {
                return 0.0f;
            }

            const uint64_t sum = std::accumulate(times->begin(), times->end(), static_cast<uint64_t>(0));
            return static_cast<float>(sum) / times->size();
        }
#endif
    };

    namespace global
    {
        inline PerfData PERF_DATA;
    }

} // namespace magique

#endif //PERFDATA_H