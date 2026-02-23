// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ENGINE_CONFIG_H
#define MAGIQUE_ENGINE_CONFIG_H

#include <raylib/raylib.h>
#include <magique/core/Types.h>
#include <magique/ui/LoadingScreen.h>
#include <magique/util/Logging.h>

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
        Theme theme{};
        Font font{};
        Timing timing;                          // Thread timing information
        Point cameraViewOff{};                  // Manual camera view offset
        Point cameraPositionOff{};              // Manual camera position offset
        LoadingScreen* loadingScreen = nullptr; // The loading screen instance
        float entityUpdateDist = 2500;          // Update distance
        float cameraCullPadding = 250;          // Padding around the cameras native bounds
        float cameraSmoothing = 0.4f;           // How fast the camera catches up to the holder position
        int benchmarkTicks = 0;                 // Ticks to run the game for
        uint16_t entityCacheDuration = 300;     // Ticks entities are still updated after they are out of range
        LogLevel logLevel = LEVEL_INFO;         // All above info are visible
        bool showPerformanceOverlay = true;     // Status of the performance overlay
        bool showPerformanceOverlayExt = true;  // Status of the extended performance overlay
        bool showEntityOverlay = false;         // Status of the entity overlay
        bool showPathFindingOverlay = false;    // Status of the pathfinding overlay
        bool showCompassOverlay = false;        // Status of the compass overlay
        bool showHitboxes = false;              // Shows red outlines for the hitboxes
        bool enableCollisionSystem = true;      // Enables the static and dynamic collision systems
        bool isClientMode = false;              // Flag to disable certain engine tasks on multiplayer clients
        bool useGameConfig = true;

        void onInit()
        {
            loadingScreen = new LoadingScreen();
            Theme adwaita{};
            adwaita.textHighlight = WHITE;
            adwaita.text = Color{192, 191, 188, 255};
            adwaita.textPassive = Color{119, 119,  119, 255};

            adwaita.backHighlight = Color{69,69,72, 255};
            adwaita.backActive = Color{46, 46, 50, 255};
            adwaita.background = Color{34, 34, 38, 255};
            adwaita.backOutline = Color{21, 21, 24, 255};

            adwaita.error = Color{246, 97, 81, 255};
            adwaita.warning = Color{255, 163, 72, 255};
            theme = adwaita;
        }
    };

    namespace global
    {
        inline Configuration ENGINE_CONFIG;
    }

} // namespace magique
#endif // MAGIQUE_ENGINE_CONFIG_H
