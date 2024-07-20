

    bool CreateGameLobby(const LobbyType type, const int maxPlayers)
    {
        SteamAPICall_t hSteamAPICall = k_uAPICallInvalid;
        switch (type)
        {
        case LobbyType::PRIVATE:
            hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePrivate, maxPlayers);
            break;
        case LobbyType::FRIENDS_ONLY:
            hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypeFriendsOnly, maxPlayers);
            break;
        case LobbyType::PUBLIC:
            hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, maxPlayers);
            break;
        }
        return hSteamAPICall != k_uAPICallInvalid;
    }