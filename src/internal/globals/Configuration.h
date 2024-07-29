#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <raylib/raylib.h>

namespace magique
{
    struct Configuration final
    {
        Font font{};                        // Font
        Rectangle worldBounds{};            // World bounds
        Vector2 shadowResolution{};         //Shadow map resolution
        float entityUpdateDistance = 1000;  // Update distance
        float cameraViewPadding = 250;      // Padding around the cameras native bounds
        int benchmarkTicks = 0;             // Ticks to run the game for
        uint16_t entityCacheDuration = 300; // Ticks entities are still updated after they are out of range
        LogLevel logLevel = LEVEL_INFO;     // All above info are visible
        LightingModel lighting = LightingModel::STATIC_SHADOWS; // Current selected lighting mode
        bool showPerformanceOverlay = true;                     // Toggles the performance overlay
        bool showHitboxes = false;                              // Shows red outlines for the hitboxes
    };

    namespace global
    {

        inline Configuration CONFIGURATION;

    }

} // namespace magique
#endif //CONFIGURATION_H