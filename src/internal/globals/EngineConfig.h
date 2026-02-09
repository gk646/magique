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

    struct Theme final
    {
        Color textActive;   // Color of primary text (selected, foreground)
        Color textSubject;  // Color of subject text (selected, category, special)
        Color textPassive;  // Color of passive text (unselected, background)
        Color backLight;    // Light background color for elements
        Color backDark;     // Dark background color for elements
        Color backSelected; // Color for elements that are selected
        Color warning;      // Color of warnings
        Color error;        // Color of errors
    };

    struct Configuration final
    {
        Theme theme{};
        Font font{};                                // Font
        Timing timing;                              // Thread timing information
        Point cameraViewOff{};                      // Manual camera view offset
        Point cameraPositionOff{};                  // Manual camera position offset
        LoadingScreen* loadingScreen = nullptr;     // The loading screen instance
        float entityUpdateDistance = 2500;          // Update distance
        float cameraCullPadding = 250;              // Padding around the cameras native bounds
        float fontSize = 15;                        // Font size of engine UI elements - scales automatically
        float cameraSmoothing = 0.4f;               // How fast the camera catches up to the holder position
        int benchmarkTicks = 0;                     // Ticks to run the game for
        uint16_t entityCacheDuration = 300;         // Ticks entities are still updated after they are out of range
        LogLevel logLevel = LEVEL_INFO;             // All above info are visible
        bool showPerformanceOverlay = true;         // Status of the performance overlay
        bool showPerformanceOverlayExt = true;      // Status of the extended performance overlay
        bool showEntityOverlay = false;             // Status of the entity overlay
        bool showPathFindingOverlay = false;        // Status of the pathfinding overlay
        bool showCompassOverlay = false;            // Status of the compass overlay
        bool showHitboxes = false;                  // Shows red outlines for the hitboxes
        bool enableCollisionSystem = true;          // Enables the static and dynamic collision systems
        bool isClientMode = false;                  // Flag to disable certain engine tasks on multiplayer clients

        void init()
        {
            loadingScreen = new LoadingScreen();
            // adwaita (Linux GNOME desktop) based colors
            Theme adwaita{};
            adwaita.textActive = WHITE;
            adwaita.textPassive = Color{163, 163, 163, 255};
            adwaita.textSubject = Color{81, 171, 250, 255};
            adwaita.backSelected = Color{68, 68, 68, 255};
            adwaita.backLight = Color{48, 48, 48, 255};
            adwaita.backDark = Color{30, 30, 30, 255};
            adwaita.error = Color{230, 97, 0, 255};
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
