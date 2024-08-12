#ifndef MAGIQUE_LOADINGSCREEN_H
#define MAGIQUE_LOADINGSCREEN_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// Loading Screen
//-----------------------------------------------
// .....................................................................
// This modules allows to draw custom loading screen. Subclass LoadingScreen and implement the loading method
// Per default this loading screen is assigned
// Note: Load method has to return true to end the loading screen
//       -> Allows loading screens longer than the load time (e.g small game, animations, context dependant, ...)
// .....................................................................

namespace magique
{
    // Sets the loading screen instance to handle different loading scenarios - see ui/LoadingScreen.h for more info
    // Note: The given pointer will be valid until a new loading screen is set
    void SetLoadingScreen(LoadingScreen* loadingScreen);

    // Returns a pointer to the current loading screen - feel free to cast to your custom type
    // Returned pointer will always be valid
    LoadingScreen* GetLoadingScreen();

    struct LoadingScreen
    {
        virtual ~LoadingScreen() = default;

        // Draws the loading screen
        // isStartup        - true if its the startup screen (e.g. showing logos and credits)
        // prograssPercent  - number in % how much loading is already done
        // IMPORTANT: MUST return true in order to end the loading screen
        virtual bool draw(bool isStartup, float prograssPercent)
        {
            DrawDefault(isStartup, prograssPercent);
            return prograssPercent >= 100.0F;
        }

    private:
        static void DrawDefault(bool isIntro, float progressPercent);
    };

} // namespace magique

#endif //MAGIQUE_LOADINGSCREEN_H