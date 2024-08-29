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

enum EntityType : uint16_t
{
    PLAYER,
    E_ARCHER,
    E_MELEE,
};

enum class MapID : uint8_t
{
    LEVEL_1 = 1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4
};

enum class StorageID
{
    ACHIEVEMENTS,
};

enum class HandleID
{
    MAPS = 0,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    MAPS_END = 10,
    TILESHEET,
    TILE_SET,
};

struct WizardQuest final : Game
{
    void onStartup(AssetLoader& loader, GameConfig& config) override;
    void onLoadingFinished() override;
    void updateGame(GameState gameState) override;
    void drawGame(GameState gameState, Camera2D& camera) override;
    void drawUI(GameState gameState) override;
    void onShutDown() override;
};

#endif // WIZARDQUEST_H