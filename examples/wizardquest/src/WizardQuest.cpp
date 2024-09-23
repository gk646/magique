#include "WizardQuest.h"

#include <raylib/raylib.h>

#include <magique/magique.hpp>
#include <magique/assets/types/TileMap.h>
#include <magique/core/StaticCollision.h>

#include "ecs/Components.h"
#include "ecs/Scripts.h"
#include "ecs/Systems.h"
#include "loading/Loaders.h"

#include <magique/core/Debug.h>


void WizardQuest::onStartup(AssetLoader& loader, GameConfig& config)
{
    // Configure raylib
    SetTargetFPS(100);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    // Configure magique
    SetShowHitboxes(true);
    InitLocalMultiplayer();
    SetStaticWorldBounds({0, 0, 1280, 1000});

    // Register loaders
    loader.registerTask(new EntityLoader(), BACKGROUND_THREAD, MEDIUM, 1);
    loader.registerTask(new TileLoader(), BACKGROUND_THREAD, MEDIUM, 3);
    loader.registerTask(new TextureLoader(), MAIN_THREAD, MEDIUM, 5);
}

void WizardQuest::onLoadingFinished()
{
    TeleportSystem::setup();
    auto map = MapID::LOBBY;
    CreateEntity(PLAYER, 24 * 24, 24 * 24, map);

    LoadGlobalTileSet(GetTileSet(HandleID::TILE_SET), {1}, 3);
    AddTileCollisions(map, GetTileMap(GetMapHandle(map)), {0, 1});

    AddTileCollisions(MapID::LEVEL_1, GetTileMap(GetMapHandle(MapID::LEVEL_1)), {0, 1});

    SetGameState(GameState::GAME);
    SetMultiplayerCallback([](MultiplayerEvent event)
    {
        printf("Event: %d\n", (int)event);
    });
}

void WizardQuest::drawGame(GameState gameState, Camera2D& camera)
{
    BeginMode2D(camera);
    const auto map = GetCameraMap();
    const auto& tileMap = GetTileMap(GetMapHandle(map));
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

    return;
    auto& pos = GetComponent<PositionC>(entt::entity(1));
    auto& col = GetComponent<CollisionC>(entt::entity(1));
    for (const auto e : GetNearbyEntities(pos.map, pos.getPosition(), 10000))
    {
        if (EntityIsActor(e))
        {
            const auto& tarPos = GetComponent<PositionC>(e);
            const auto& tarCol = GetComponent<CollisionC>(e);
            if (tarPos.map != pos.map)
                break;
            std::vector<Point> path;
            StartTimer(0);
            FindPath(path, pos.getMiddle(col), tarPos.getMiddle(tarCol), pos.map, 64);
            printf("Micros: %d\n", StopTimer(0) / 1000);
            DrawPath(path);
        }
    }
    DrawPathFindingGrid(map);
    EndMode2D();
}

void WizardQuest::drawUI(GameState gameState)
{
    switch (gameState)
    {
    case GameState::MAIN_MENU:
        break;
    case GameState::GAME:
       // hudd.render();
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
        TeleportSystem::update();
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


}

void WizardQuest::onShutDown()
{
    GameSave save;
    auto data = GetAchievementsData();
    save.saveData(StorageID::ACHIEVEMENTS, data.getData(), data.getSize());
    GameSave::Save(save, "MySave.save");
}