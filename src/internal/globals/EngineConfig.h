#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <raylib/raylib.h>

namespace magique
{


    struct Configuration final
    {
        Font font{};             // Font
        Rectangle worldBounds{}; // World bounds
        struct Timing final
        {
            double frameTarget = 0.0F; // How long a single frame can take at maximum
            double sleepTime = 0.0F;   // How long to sleep of the total wait time (sleep accuracy is at best 1ms)
            double workPerTick = 0.0F; // How much of an update has to happen per render tick
            int frameCounter = 0;      // Current frames counter
        } timing;
        Vector2 manualCamOff{0, 0};                 // Manual camera offset
        float entityUpdateDistance = 1000;          // Update distance
        float cameraViewPadding = 250;              // Padding around the cameras native bounds
        int benchmarkTicks = 0;                     // Ticks to run the game for
        uint16_t entityCacheDuration = 300;         // Ticks entities are still updated after they are out of range
        LogLevel logLevel = LEVEL_INFO;             // All above info are visible
        LightingMode lighting = LightingMode::NONE; // Current selected lighting mode
        bool showPerformanceOverlay = true;         // Toggles the performance overlay
        bool showHitboxes = false;                  // Shows red outlines for the hitboxes

        void init()
        {
            font = GetFontDefault();
            if (font.texture.id == 0)
            {
                LOG_ERROR("Failed to load default font");
                LOG_ERROR("Failed to initialize magique");
            }
        }
    };

    namespace global
    {

        inline Configuration ENGINE_CONFIG;

    }

} // namespace magique
#endif //CONFIGURATION_H