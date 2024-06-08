#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <raylib/raylib.h>

namespace magique{

    struct Configuration final
    {
        // Current selected lighting mode
        LightingModel lighting = LightingModel::STATIC_SHADOWS;

        // Toggles the performance overlay
        bool showPerformanceOverlay = true;

        // All above info are visible
        LogLevel logLevel = LogLevel::LEVEL_INFO;

        // Update distance
        float entityUpdateDistance = 1000;

        float cameraViewPadding = 250;

        // For how long entities in the cache are still updates after they are out of range
        uint16_t entityCacheDuration = 300; // 300 Ticks -> 5 seconds

        // Font
        Font font{};
    };


    namespace global{


    inline Configuration CONFIGURATION;



    }}
#endif //CONFIGURATION_H