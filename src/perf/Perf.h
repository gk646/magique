#ifndef PROFILE_H
#define PROFILE_H

#include <cstdint>
#include <numeric>
#include <vector>

#include "ui/overlay/PerformanceDisplay.h"

enum TickType
{
    UPDATE,
    DRAW
};


struct PerfData
{
    PerformanceDisplay perfOverlay;
    uint32_t logicTickTime = 0;
    uint32_t drawTickTime = 0;
#if MAGIQUE_DEBUG == 1
    std::vector<uint32_t> logicTimes;
    std::vector<uint32_t> drawTimes;
#endif

    PerfData()
    {
#if MAGIQUE_DEBUG == 1
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
#if MAGIQUE_DEBUG == 1
            logicTimes.push_back(time);
#endif
        }
        else if (t == DRAW)
        {
            drawTickTime = time;
#if MAGIQUE_DEBUG == 1
            drawTimes.push_back(time);
#endif
        }
    }

    // Dont even let it be there
#if MAGIQUE_DEBUG == 1
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


#endif //PROFILE_H