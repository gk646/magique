#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <raylib/raylib.h>

namespace magique
{
    struct Configuration final
    {
        //Shadow map resolution
        Vector2 shadowResolution{};

        // Current selected lighting mode
        LightingModel lighting = LightingModel::STATIC_SHADOWS;

        // Toggles the performance overlay
        bool showPerformanceOverlay = true;

        // All above info are visible
        LogLevel logLevel = LEVEL_INFO;

        // Update distance
        float entityUpdateDistance = 1000;

        // Padding around the cameras native bounds
        float cameraViewPadding = 250;

        // For how long entities in the cache are still updates after they are out of range
        uint16_t entityCacheDuration = 300; // 300 Ticks -> 5 seconds

        // Font
        Font font{};
    };

    namespace global
    {

        inline Configuration CONFIGURATION;

    }

} // namespace magique
#endif //CONFIGURATION_H