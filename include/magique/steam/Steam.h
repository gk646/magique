// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STEAM_H
#define MAGIQUE_STEAM_H

#include <magique/core/Types.h>
#include <string_view>
#include <functional>

//===============================================
// Steam Module
//===============================================
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module is a wrapper for the steam sdk that allows easier and structured access to steam functionalities
// .....................................................................

namespace magique
{
    // Returns true if the initialization of steam was successful
    // If specified creates a test steam_appid.txt file with the id 480 (test project)
    bool SteamInit(bool createAppIDFile = true);

    // Returns the value of the launch parameter with the given key the game was launched with
    // Note: Check out https://partner.steamgames.com/doc/api/ISteamApps#GetLaunchCommandLine
    std::string_view SteamGetLaunchParam(std::string_view key);

    //================= USER =================//

    // Returns your own steam id
    SteamID SteamGetID();

    // Returns the name of this steam user
    const char* SteamGetUserName();

    // Returns the name of the user with the given id (only works if it's your friend or in a common lobby etc.)
    const char* SteamGetName(SteamID id);

    using SteamOverlayCallback = std::function<void(bool isOpening)>;

    // Sets the callback function called when the steam overlay is opened or closed
    void SteamSetOverlayCallback(const SteamOverlayCallback& callback);

    //================= PERSISTENCE =================//

    // Returns the path of the local user data folder specific to the game and steam id
    // Note: This should be used to load and store local data for steam games - automatically adjusts to game and user
    const char* SteamGetUserDataLocation();

} // namespace magique


#endif // MAGIQUE_STEAM_H
