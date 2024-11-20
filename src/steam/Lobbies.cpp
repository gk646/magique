#define _CRT_SECURE_NO_WARNINGS
#include <magique/steam/Lobbies.h>
#include <magique/internal/Macros.h>

#include "internal/globals/SteamData.h"

namespace magique
{
    bool CreateSteamLobby(const LobbyType type, const int maxPlayers)
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
        auto& steamData = global::STEAM_DATA;
        steamData.m_SteamCallResultCreateLobby.Set(hSteamAPICall, &steamData, &SteamData::OnLobbyCreated);
        return hSteamAPICall != k_uAPICallInvalid;
    }

    void JoinSteamLobby(LobbyID lobbyID)
    {
        MAGIQUE_ASSERT(!IsInLobby(), "Cant join a lobby when in a lobby");
        SteamMatchmaking()->JoinLobby(CSteamID(static_cast<uint64>(lobbyID)));
    }

    bool LeaveSteamLobby()
    {
        if (!IsInLobby())
            return false;
        const auto& steam = global::STEAM_DATA;
        SteamMatchmaking()->LeaveLobby(steam.lobbyID);
        if (steam.lobbyEventCallback)
        {
            const auto lobbyID = LobbyIDFromSteam(steam.lobbyID);
            const auto steamID = MagiqueIDFromSteam(steam.userID);
            steam.lobbyEventCallback(lobbyID, steamID, LobbyEvent::ON_LOBBY_EXIT);
        }
        return true;
    }

    bool IsInLobby() { return global::STEAM_DATA.lobbyID.IsValid(); }

    bool IsLobbyOwner()
    {
        const auto& steamData = global::STEAM_DATA;
        if (IsInLobby())
        {
            const auto owner = SteamMatchmaking()->GetLobbyOwner(steamData.lobbyID);
            return owner == steamData.userID;
        }
        return false;
    }

    LobbyID GetLobbyID() { return static_cast<LobbyID>(global::STEAM_DATA.lobbyID.ConvertToUint64()); }

    SteamID GetLobbyOwner()
    {
        MAGIQUE_ASSERT(IsInLobby(), "Cant get the lobby owner when not in a lobby");
        return static_cast<SteamID>(SteamMatchmaking()->GetLobbyOwner(global::STEAM_DATA.lobbyID).ConvertToUint64());
    }

    void SetLobbyEventCallback(const LobbyCallback& lobbyEventCallback)
    {
        auto& steamData = global::STEAM_DATA;
        steamData.lobbyEventCallback = lobbyEventCallback;
    }

    bool OpenLobbyInviteDialogue()
    {
        auto& steam = global::STEAM_DATA;
        if (IsInLobby())
        {
            SteamFriends()->ActivateGameOverlayInviteDialog(steam.lobbyID);
            return true;
        }
        return false;
    }

    bool InviteUserToLobby(SteamID userID)
    {
        MAGIQUE_ASSERT(IsInLobby(), "Cant invite others to lobby if not in a lobby");
        return SteamMatchmaking()->InviteUserToLobby(global::STEAM_DATA.lobbyID, SteamIDFromMagique(userID));
    }

    void SendLobbyChatMessage(const char* message)
    {
        MAGIQUE_ASSERT(IsInLobby(), "Cant send a chat message when not in a lobby");
        MAGIQUE_ASSERT(message != nullptr, "passed null msg");
        const int messageLength = static_cast<int>(strlen(message));
        SteamMatchmaking()->SendLobbyChatMsg(global::STEAM_DATA.lobbyID, message, messageLength);
    }

    void SetChatMessageCallback(const ChatCallback& chatCallback)
    {
        auto& steamData = global::STEAM_DATA;
        steamData.chatCallback = chatCallback;
    }

    //----------------- LOBBY IMPLEMENTATION -----------------//

    void SteamData::OnLobbyChatUpdate(LobbyChatUpdate_t* pCallback) const
    {
        if (!lobbyEventCallback || pCallback->m_ulSteamIDLobby != lobbyID.ConvertToUint64())
            return;

        const auto steamID = static_cast<SteamID>(pCallback->m_ulSteamIDUserChanged);
        const auto lobbyID = LobbyIDFromSteam(global::STEAM_DATA.lobbyID);
        if ((pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered) != 0)
        {
            MAGIQUE_ASSERT(pCallback->m_ulSteamIDUserChanged != userID.ConvertToUint64(), "Our event already fired?");
            lobbyEventCallback(lobbyID, steamID, LobbyEvent::ON_USER_JOINED);
        }
        else if ((pCallback->m_rgfChatMemberStateChange & k_EChatMemberStateChangeLeft) != 0)
        {
            MAGIQUE_ASSERT(pCallback->m_ulSteamIDUserChanged != userID.ConvertToUint64(), "Our event already fired?");
            lobbyEventCallback(lobbyID, steamID, LobbyEvent::ON_USER_LEFT);
        }
    }

    void SteamData::OnLobbyDataUpdate(LobbyDataUpdate_t* pCallback) const
    {
        if (pCallback->m_ulSteamIDLobby == lobbyID.ConvertToUint64())
        {
            // LOG_INFO("Lobby data updated");
        }
    }

    void SteamData::OnLobbyChatMessage(LobbyChatMsg_t* pCallback)
    {
        if (pCallback->m_ulSteamIDLobby == lobbyID.ConvertToUint64())
        {
            CSteamID senderID;
            EChatEntryType chatEntryType;
            char message[MAGIQUE_MAX_LOBBY_MESSAGE_LENGTH];
            const auto chatId = static_cast<int>(pCallback->m_iChatID);
            const int messageSize = SteamMatchmaking()->GetLobbyChatEntry(
                lobbyID, chatId, &senderID, message, MAGIQUE_MAX_LOBBY_MESSAGE_LENGTH, &chatEntryType);

            if (messageSize > 0 && chatEntryType == k_EChatEntryTypeChatMsg)
            {
                cacheString = message;
                if (chatCallback)
                    chatCallback(static_cast<SteamID>(senderID.ConvertToUint64()), cacheString);
            }
        }
    }

    void SteamData::OnLobbyEntered(LobbyEnter_t* pCallback) const
    {
        auto& steam = global::STEAM_DATA;
        steam.lobbyID = pCallback->m_ulSteamIDLobby;
        if (lobbyEventCallback)
        {
            const auto lobbyID = LobbyIDFromSteam(steam.lobbyID);
            const auto steamID = MagiqueIDFromSteam(steam.userID);
            lobbyEventCallback(lobbyID, steamID, LobbyEvent::ON_LOBBY_ENTERED);
        }
    }

    void SteamData::OnLobbyCreated(LobbyCreated_t* pCallback, bool /**/)
    {
        auto& steam = global::STEAM_DATA;
        if (pCallback->m_eResult == k_EResultOK)
        {
            steam.lobbyID = pCallback->m_ulSteamIDLobby;
            if (lobbyEventCallback)
            {
                const auto lobbyID = LobbyIDFromSteam(steam.lobbyID);
                const auto steamID = MagiqueIDFromSteam(steam.userID);
                lobbyEventCallback(lobbyID, steamID, LobbyEvent::ON_LOBBY_CREATED);
            }
        }
    }


} // namespace magique