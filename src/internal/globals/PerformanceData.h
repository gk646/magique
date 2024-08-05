#ifndef PERFDATA_H
#define PERFDATA_H

#include <magique/util/Defines.h>
#include "ui/internal/PerformanceDisplay.h"
#include "internal/datastructures/VectorType.h"

namespace magique
{
    enum TickType
    {
        UPDATE,
        DRAW
    };

    struct PerformanceData final
    {
        uint32_t logicTickTime = 0;
        uint32_t drawTickTime = 0;
        PerformanceDisplay perfOverlay;
#ifdef MAGIQUE_DEBUG_PROFILE
        vector<uint32_t> logicTimes;
        vector<uint32_t> drawTimes;
#endif

        PerformanceData()
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
        [[nodiscard]] float getAverageTime(const TickType t)
        {
            vector<uint32_t>* times;
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
            times->pop_back();

            uint64_t sum = 0;
            for (const auto num : *times)
            {
                sum += num;
            }
            return static_cast<float>(sum) / static_cast<float>(times->size());
        }
#endif
    };

    namespace global
    {
        inline PerformanceData PERF_DATA;
    }

} // namespace magique

#endif //PERFDATA_H