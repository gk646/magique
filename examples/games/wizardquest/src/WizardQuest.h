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
    POSITION_UPDATE,
    INPUT_UPDATE,
    SPAWN_UPDATE,
};

enum EntityType : uint16_t
{
    PLAYER,
    TROLL,      // Enemy
    NET_PLAYER, // Other players connected over network
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
    ACHIEVEMENTS, // Storage slot for the achievements
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
    WizardQuest() : Game("WizardQuest") {}
    void onStartup(AssetLoader& loader) override;
    void onLoadingFinished() override;
    void updateGame(GameState gameState) override;
    void postTickUpdate(GameState gameState) override;
    void drawGame(GameState gameState, Camera2D& camera) override;
    void drawUI(GameState gameState) override;
    void onShutDown() override;
};

#endif // WIZARDQUEST_H