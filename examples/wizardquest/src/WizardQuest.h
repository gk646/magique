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

struct WizardQuest final : Game
{
    void onStartup(AssetLoader &loader, GameConfig &config) override;
    void updateGame(GameState gameState) override;
    void drawGame(GameState gameState) override;
};

#endif // WIZARDQUEST_H