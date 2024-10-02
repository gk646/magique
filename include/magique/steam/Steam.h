#ifndef MAGIQUE_STEAM_H
#define MAGIQUE_STEAM_H

//-----------------------------------------------
// Steam Module
//-----------------------------------------------
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module is a wrapper for the steam sdk that allows easier and structured access to steam functionalities
// .....................................................................

namespace magique
{

    // Returns the location of the local user data folder specific to the game and steam id
    // Note: This should be used to load and store local data for steam games - automatically adjusts to game and user
    const char* GetUserDataLocation();


    //----------------- FRIENDS -----------------//




} // namespace magique



#endif //MAGIQUE_STEAM_H