#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <raylib/raylib.h>

namespace magique
{
    struct Configuration final
    {
        Font font{};                                // Font
        Rectangle worldBounds{};                    // World bounds
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

        inline Configuration CONFIGURATION;

    }

} // namespace magique
#endif //CONFIGURATION_H