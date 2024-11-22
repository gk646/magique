#ifndef PERSISTENCEDEMO_H
#define PERSISTENCEDEMO_H

#include <magique/core/Game.h>
#include <magique/persistence/GameLoader.h>
#include <magique/persistence/GameSaver.h>

using namespace magique;

GameSaver GAME_SAVER;

struct PersistenceDemo final : Game
{
    void onStartup(AssetLoader& loader) override {}

    void onLoadingFinished() override {}

    void updateGame(GameState gameState) override {}

    void drawGame(GameState gameState, Camera2D& camera2D) override {}
};

#endif //PERSISTENCEDEMO_H