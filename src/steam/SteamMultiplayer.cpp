#include <magique/core/Types.h>

#include "internal/globals/SteamData.h"

namespace magique
{
    Connection ConnectToSocket(const SteamID steamID)
    {
        auto& data = global::STEAM_DATA.networkingData;
        SteamNetworkingIdentity id{};
        id.SetSteamID({(uint64_t)steamID});
        data.connections[0] = SteamNetworkingSockets()->ConnectP2P(id, 0, 0, nullptr);
        data.isHost = false;
        return static_cast<Connection>(data.connections[0]);
    }



} // namespace magique