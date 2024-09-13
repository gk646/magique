#include "WizardQuest.h"

#include "../../../src/external/cxstructs/cxutil/cxtime.h"

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
    auto map = MapID::LEVEL_2;
    CreateEntity(PLAYER, 23*24 ,3* 24, map);
    CreateEntity(TROLL, 23*24, 30*24, map);
    // LoadMapColliders(map, GetTileMap(HandleID(map)).getObjects(0),3);
    LoadGlobalTileSet(GetTileSet(HandleID::TILE_SET), {1}, 3);
    LoadTileMapCollisions(map, GetTileMap(HandleID(map)), {0, 1});
    SetGameState(GameState::GAME);
}

void WizardQuest::drawGame(GameState gameState, Camera2D& camera)
{
    BeginMode2D(camera);
    const auto map = GetCameraMap();
    const auto& tileMap = GetTileMap(HandleID::LEVEL_1);
    DrawTileMap(tileMap, GetTileSheet(HandleID::TILESHEET), 0);
    DrawTileMap(tileMap, GetTileSheet(HandleID::TILESHEET), 1);
    for (const auto entity : GetDrawEntities())
    {
        if (EntityHasComponents<AnimationC>(entity))
        {
            const auto& pos = GetComponent<PositionC>(entity);
            const auto& anim = GetComponent<AnimationC>(entity);
            const auto& mov = GetComponent<MovementC>(entity);
            anim.drawCurrentFrame(pos.x, pos.y, 0, mov.movedLeft);
        }
    }

    auto& pos = GetComponent<PositionC>(entt::entity(1));
    auto& col = GetComponent<CollisionC>(entt::entity(1));
    for (const auto e : GetNearbyEntities(pos.getPosition(), 10000))
    {
        if (EntityIsActor(e))
        {
            const auto& tarPos = GetComponent<PositionC>(e);
            const auto& tarCol = GetComponent<CollisionC>(e);
            if (tarPos.map != pos.map)
                break;
            std::vector<Point> path;
            cxstructs::now();
            FindPath(path, pos.getMiddle(col), tarPos.getMiddle(tarCol), pos.map,0);
            cxstructs::printTime<std::chrono::nanoseconds>("Time:");
            DrawPath(path);
        }
    }
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
        AnimationSystem::update();
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