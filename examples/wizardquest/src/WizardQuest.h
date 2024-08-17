#ifndef WIZARDQUEST_H
#define WIZARDQUEST_H

#include <magique/core/Game.h>

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

enum EntityID : uint16_t
{
    PLAYER,
    E_ARCHER,
    E_MELEE,
};

enum class MapID : uint8_t
{
    LEVEL_1,
    LEVEL_2
};

enum class StorageID
{
    ACHIEVEMENTS,
};

struct WizardQuest final : Game
{
    void onStartup(AssetLoader& loader, GameConfig& config) override;
    void updateGame(GameState gameState) override;
    void drawGame(GameState gameState, Camera2D& camera) override;
    void drawUI(GameState gameState) override;
    void onShutDown() override;
};


#endif // WIZARDQUEST_H