#ifndef MAGIQUE_MULTIPLAYER_H
#define MAGIQUE_MULTIPLAYER_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Multiplayer Module
//-----------------------------------------------
// .....................................................................
// Currently only supports the Steamworks SDK backend
// This multiplayer module allows for secure Peer-to-Peer multiplayer over the steam relay network
// even with NAT traversal. Its setup so that the hosting peer is the server and a player at the same time
// while other peers are regular clients. This is great for non ultra-competitive games with moderate lobby sizes.
// .....................................................................

namespace magique
{

    // Initializes multiplayer - HAS to be called MANUALLY before any other multiplayer methods
    // Automatically creates the test steam_appid.txt file with id 480
    bool InitMultiplayer();


    //----------------- LOBBIES -----------------//

    // Returns true if the async call trying to create a lobby was successful
    bool CreateGameLobby(LobbyType type, int maxPlayers);


} // namespace magique

#endif //MAGIQUE_MULTIPLAYER_H