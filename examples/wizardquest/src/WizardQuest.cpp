#include "WizardQuest.h"

#include <raylib/raylib.h>

#include <magique/magique.hpp>

#include "ecs/Components.h"
#include "ecs/Scripts.h"
#include "ecs/Systems.h"
#include "ui/UiScenes.h"
#include "loading/Loaders.h"

#include <magique/assets/types/TileMap.h>
#include <magique/core/StaticCollision.h>

PlayerHUD hudd{};

void WizardQuest::onStartup(AssetLoader& loader, GameConfig& config)
{
    // Configure raylib
    SetTargetFPS(100);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    // Configure magique
    SetShowHitboxes(true);
    InitLocalMultiplayer();
    // SetStaticWorldBounds({0,0,1280,1000});

    // Register loaders
    loader.registerTask(new EntityLoader(), BACKGROUND_THREAD, MEDIUM, 1);
    loader.registerTask(new TileLoader(), BACKGROUND_THREAD, MEDIUM, 3);
    loader.registerTask(new TextureLoader(), MAIN_THREAD, MEDIUM, 5);
}

void WizardQuest::onLoadingFinished()
{
        CreateEntity(PLAYER, 55, 55, MapID::LEVEL_3);
    auto map = GetCameraMap();

    LoadMapColliders(map, GetTileMap(HandleID(map)).getObjects(0),3);
    LoadGlobalTileSet(GetTileSet(HandleID::TILE_SET),{1},48.0F);
    LoadTileMap(map,GetTileMap(HandleID(map)),{0,1});
    SetGameState(GameState::GAME);
}

void WizardQuest::drawGame(GameState gameState, Camera2D& camera)
{
    BeginMode2D(camera);
    auto map = GetCameraMap();
    const auto& tileMap = GetTileMap(GetHandle(HandleID((int)HandleID::MAPS + (int)map)));
    DrawTileMap(tileMap, GetTileSheet(GetHandle(HandleID::TILESHEET)), 0);
    DrawTileMap(tileMap, GetTileSheet(GetHandle(HandleID::TILESHEET)), 1);
    DrawHashGridDebug();
    EndMode2D();
}

void WizardQuest::drawUI(GameState gameState)
{
    switch (gameState)
    {
    case GameState::MAIN_MENU:
        break;
    case GameState::GAME:
        hudd.render();
        break;
    case GameState::GAME_OVER:
        break;
    }
}

void WizardQuest::updateGame(GameState gameState)
{
    switch (gameState)
    {
    case GameState::MAIN_MENU:
        break;
    case GameState::GAME:
        MovementSystem::update();
        break;
    case GameState::GAME_OVER:
        break;
    }

    auto& messages = ReceiveMessages();
    for (auto& msg : messages)
    {
        switch (msg.payload.type)
        {
        case MessageType::STRING:
            printf("Msg: %s", (const char*)msg.payload.data);
            break;
        }
    }

    if (IsKeyPressed(KEY_J))
    {
        if (IsHost())
        {
        }
    }

}

void WizardQuest::onShutDown()
{
    GameSave save;
    auto data = GetAchievementsData();
    save.saveData(StorageID::ACHIEVEMENTS, data.getData(), data.getSize());
    GameSave::Save(save, "MySave.save");
}