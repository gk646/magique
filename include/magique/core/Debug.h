#ifndef MAGIQUE_DEBUG_H
#define MAGIQUE_DEBUG_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// Debug Module
//-----------------------------------------------
// ................................................................................
// This module allows to enable/disable debug functionality
// ................................................................................

namespace magique
{
    // If enabled display performance metrics on the top left
    // Default: false
    void SetShowPerformanceOverlay(bool val);

    // If true shows red hitboxes for collidable entities - can get very expensive
    // This is the single point of truth - If two hitboxes visually overlap then a collision happened!
    void SetShowHitboxes(bool val);

    // Sets the amount of logic ticks until the game closes automatically
    // This ensures same length benchmarks
    // Default: 0 - off
    void SetBenchmarkTicks(int ticks);

    // Clears the collected benchmark times
    void ResetBenchmarkTimes();

    // Draws the debug grid for the dynamic entity hash grid including entity count
    void DrawHashGridDebug(MapID map);

    //----------------- TIME MEASURE -----------------//

    // Starts a timer from 0 with the given id
    void StartTimer(int num);

    // Stops the timer with the given id and returns elapsed nanos since starting
    int StopTimer(int num);

} // namespace magique
#endif //MAGIQUE_DEBUG_H