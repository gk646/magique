// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_DEBUG_H
#define MAGIQUE_DEBUG_H

#include <magique/fwd.hpp>

//===============================================
// Debug Module
//===============================================
// ................................................................................
// This module allows to enable/disable certain debug functionality
// ................................................................................

namespace magique
{
    // If enabled, displays metrics on the top left:
    //      - FPS: frames per second (smoothed)
    // If extended:
    //      - CPU: duration of the last update tick (millis)
    //      - GPU: duration of the last draw tick (millis)
    //      - Draw Calls: how many draw calls have been made to the GPU (does not work for custom OpenGL calls)
    //
    // Default: true, true
    void SetShowPerformanceOverlay(bool val, bool extended = true);

    // If true shows red hitboxes for collideable entities - can get very expensive if there are a lot of entities.
    // Note: This is the single point of truth - if two hitboxes visually overlap then a collision happened! (else a bug)
    void SetShowHitboxes(bool val);

    // If enabled, draws the debug grid for the dynamic entity hash grid including entity count - drawn before user content
    // Note: draws the state of the map returned by GetCameraMap() (needs an active camera)
    void SetShowEntityGridOverlay(bool val);

    // If enabled, draws the current state of the pathfinding grid
    // Transparent (not solid), grey (solid from static source), black (solid from dynamic source)
    // Note: draws the state of the map returned by GetCameraMap() (needs an active camera)
    void SetShowPathFindingOverlay(bool val);

    // If enabled, draws a cartesian coordinate system with markings every 250 pixels - drawn before user content
    void SetShowCompassOverlay(bool val);

    // Sets the amount of logic ticks until the game closes automatically - this ensures the same run length for benchmarks
    // Default: 0 (off)
    void SetBenchmarkTicks(int ticks);

    // Clears the collected performance data collected so far
    void ResetPerformanceData();

    //================= TIME MEASURE =================//

    // Starts a timer from 0 with the given id
    void StartTimer(int num);

    // Returns the current time passed since the timer was started in nanoseconds
    // Failure: Returns UINT64_MAX if no timer was started in this slot
    uint64_t GetTimerTime(int num);

    // Stops the timer with the given id and returns elapsed nanos since starting
    uint64_t StopTimer(int num);

} // namespace magique

#endif //MAGIQUE_DEBUG_H