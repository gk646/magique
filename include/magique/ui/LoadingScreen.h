// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_LOADINGSCREEN_H
#define MAGIQUE_LOADINGSCREEN_H

#include <magique/fwd.hpp>

//===============================================
// Loading Screen
//===============================================
// .....................................................................
// This modules allows to draw custom loading screen. Subclass LoadingScreen and implement the loading method
// If you don't set a custom LoadingScreen an instance of this default one is assigned
// Note: draw() method has to return true in order to end the loading screen
//       -> Allows loading screens longer than the load time (e.g. small game, animations, context dependant, ...)
// .....................................................................

namespace magique
{
    // Sets the loading screen instance to handle different loading scenarios
    // Note: The given pointer will be valid until a new loading screen is set
    void LoadingScreenSet(LoadingScreen* loadingScreen);

    // Returns a pointer to the current loading screen - feel free to cast to your custom type
    // Returned pointer will always be valid
    LoadingScreen* LoadingScreenGet();

    struct LoadingScreen
    {
        virtual ~LoadingScreen() = default;

        // Draws the loading screen
        // isStartup        - true if it's the startup loading screen (e.g. should show lgos and credits)
        // progressPercent  - number in % how much loading is already done
        // IMPORTANT: MUST return true in order to end the loading screen
        virtual bool draw(bool isStartup, float progressPercent)
        {
            DrawDefault(isStartup, progressPercent);
            return progressPercent >= 100.0F;
        }

    private:
        static void DrawDefault(bool isIntro, float progressPercent);
    };

} // namespace magique

#endif //MAGIQUE_LOADINGSCREEN_H