#ifndef MAGIQUE_STEAM_H
#define MAGIQUE_STEAM_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Steam Module
//-----------------------------------------------
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module is a wrapper for the steam sdk that allows easier and structured access to steam functionalities
// .....................................................................

namespace magique
{

    // Returns true if the initialization of steam was successful
    // If specified creates a test steam_appid.txt file with the id 480 (test project)
    bool InitSteam(bool createAppIDFile);

    //----------------- USER -----------------//

    // Returns your own steam id
    SteamID GetUserSteamID();

    // Returns the name of this steam user - copy the string if you want to save it
    const char* GetSteamUserName();

    // Sets the callback function called when the steam overlay is opened or closed
    void SetSteamOverlayCallback(SteamOverlayCallback steamOverlayCallback);

    //----------------- PERSISTENCE -----------------//

    // Returns the location of the local user data folder specific to the game and steam id
    // Note: This should be used to load and store local data for steam games - automatically adjusts to game and user
    const char* GetUserDataLocation();


} // namespace magique


#endif //MAGIQUE_STEAM_H