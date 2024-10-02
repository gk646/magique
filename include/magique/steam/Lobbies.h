#ifndef MAGIQUE_STEAM_LOBBIES_H
#define MAGIQUE_STEAM_LOBBIES_H

//-----------------------------------------------
// Steam Lobbies Module
//-----------------------------------------------
// .....................................................................
// Note: This module needs steam to be enabled (via CMake: MAGIQUE_STEAM)
// This module is allows to create
// .....................................................................

namespace magique{

// Returns true if the async call trying to create a lobby was successful
bool CreateGameLobby(int type, int maxPlayers);

}


#endif //MAGIQUE_STEAM_LOBBIES_H