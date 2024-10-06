#ifndef WIZARDQUEST_H
#define WIZARDQUEST_H

#include <magique/core/Game.h>
#include "ui/UIControls.h"

using namespace magique; // using namespace is recommended and allowed

enum class GameState
{
    MAIN_MENU,
    GAME,
    GAME_OVER
};

enum class MessageType : uint8_t
{
    STRING,
};

enum EntityType : uint16_t
{
    PLAYER,
    TROLL,
};

enum class MapID : uint8_t
{
    LOBBY,
    LEVEL_1,
    LEVEL_2,
    MAPS_END,
};

enum class StorageID
{
    ACHIEVEMENTS,
};

enum class HandleID
{
    MAPS_START = 10,
    LOBBY,
    LEVEL_1,
    LEVEL_2,
    TILESHEET,
    TILE_SET,
};

inline HandleID GetMapHandle(MapID map) { return HandleID((int)HandleID::MAPS_START + (int)map + 1); }

enum class AnimationState : uint8_t
{
    IDLE,
    RUN,
    JUMP,
    ATTACK_1,
    ATTACK_2,
};

enum class AtlasID : int
{
    DEFAULT, // 0 is used as default
    ENTITIES,
};

struct WizardQuestUI final
{
    PlayerHUD playerHUD{};
    LobbyBrowser lobbyBrowser{};
    PlayerHotbar playerHotbar{};
};

struct WizardQuest final : Game
{
    WizardQuestUI gameUI{};
    void onStartup(AssetLoader& loader) override;
    void onLoadingFinished() override;
    void updateGame(GameState gameState) override;
    void drawGame(GameState gameState, Camera2D& camera) override;
    void drawUI(GameState gameState) override;
    void onShutDown() override;
};

#endif // WIZARDQUEST_H