#ifndef MAGIQUE_ENGINE_CONFIGU_H
#define MAGIQUE_ENGINE_CONFIGU_H

#include <raylib/raylib.h>

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <magique/ui/LoadingScreen.h>

namespace magique
{
    struct Timing final
    {
        double frameTarget = 0.0F; // How long a single frame can take at maximum
        double sleepTime = 0.0F;   // How long to sleep of the total wait time (sleep accuracy is at best 1ms)
        double workPerTick = 0.0F; // How much of an update has to happen per render tick
        int frameCounter = 0;      // Current frames counter
    };

    struct Configuration final
    {
        Font font{};                                // Font
        Rectangle worldBounds{};                    // World bounds
        Timing timing;                              // Thread timing information
        Vector2 manualCamOff{0, 0};                 // Manual camera offset
        LoadingScreen* loadingScreen = nullptr;     // The loading screen instance
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
            loadingScreen = new LoadingScreen();
        }

        [[nodiscard]] bool getIsWorldBoundSet() const { return worldBounds.width != 0.0F; }
    };

    namespace global
    {
        inline Configuration ENGINE_CONFIG;
    }

} // namespace magique
#endif //MAGIQUE_ENGINE_CONFIGU_H