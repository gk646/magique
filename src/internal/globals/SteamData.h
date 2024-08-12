#ifndef MAGIQUE_STEAMDATA_H
#define MAGIQUE_STEAMDATA_H

#include <steam/steam_api.h>
#include <cxstructs/SmallVector.h>

#include <magique/util/Defines.h>

namespace magique
{
    using MessageBuffer = cxstructs::SmallVector<SteamNetworkingMessage_t*, MAGIQUE_MESSAGES_ESTIMATE>;

    struct NetworkingData final
    {
        bool isHost;
        HSteamListenSocket listenSocket = k_HSteamListenSocket_Invalid;
        HSteamNetConnection connections[MAGIQUE_MAX_PLAYERS]{};
        MessageBuffer messages;
    };

    struct SteamData final
    {
        NetworkingData networkingData;
    };

    namespace global
    {
        inline SteamData STEAM_DATA{};
    }
} // namespace magique

#endif //MAGIQUE_STEAMDATA_H