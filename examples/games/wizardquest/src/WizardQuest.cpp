#include "WizardQuest.h"

#include <magique/magique.hpp> // Single include header

#include "ecs/Components.h"
#include "ecs/Scripts.h"
#include "ecs/Systems.h"
#include "loading/Loaders.h"
#include "multiplayer/Multiplayer.h"

void WizardQuest::onStartup(AssetLoader& loader)
{
    // Configure raylib
    SetTargetFPS(100);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    // Configure magique
    SetShowHitboxes(true);

    // Set static world bounds
    SetStaticWorldBounds({0, 0, 1280, 1000});

    // Register loaders
    loader.registerTask(new EntityLoader(), THREAD_ANY, MEDIUM, 1);
    loader.registerTask(new TileLoader(), THREAD_ANY, MEDIUM, 3);
    loader.registerTask(new TextureLoader(), THREAD_MAIN, MEDIUM, 5);
}

void WizardQuest::onLoadingFinished()
{
    TeleportSystem::setup();
    Multiplayer::init();

    // Set the initial map
    auto map = MapID::LOBBY;
    // Create the player
    CreateEntity(PLAYER, 24 * 24, 24 * 24, map);

    // Load the global tileset - the tileset defines the collision (and other) attributes for tiles
    // Mark all tiles with class 1 as solid
    LoadGlobalTileSet(GetTileSet(HandleID::TILE_SET), {1}, 3);

    // Adds all solid tiles from the given map as static collision objects - from layer 0 and 1
    AddTileCollisions(map, GetTileMap(GetMapHandle(map)), {0, 1});
    AddTileCollisions(MapID::LEVEL_1, GetTileMap(GetMapHandle(MapID::LEVEL_1)), {0, 1});

    // Start the game in game state
    SetGameState(GameState::GAME);
}

void WizardQuest::drawGame(GameState gameState, Camera2D& camera)
{
    BeginMode2D(camera);
    {
        // Get the current map
        const auto map = GetCameraMap();
        // Get the map data from the asset manager
        const auto& tileMap = GetTileMap(GetMapHandle(map));
        // Draw the specified layer of the given map using the textures from the given tilesheet
        DrawTileMap(tileMap, GetTileSheet(HandleID::TILESHEET), 0);
        DrawTileMap(tileMap, GetTileSheet(HandleID::TILESHEET), 1);

        // Draw the entities using their defined animation data
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
        gameUI.playerHUD.draw();
        gameUI.playerHotbar.draw();
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
        Multiplayer::update();
        MovementSystem::update();
        AnimationSystem::update();
        TeleportSystem::update();
        break;
    case GameState::GAME_OVER:
        break;
    }
}

// Update happens after the internal update tick - we want to send out the most up-to-date position for entities
void WizardQuest::postTickUpdate(GameState gameState) { Multiplayer::postUpdate(); }

// Runs once on shutdown - save our game data
void WizardQuest::onShutDown()
{
    GameSaveData save;
    auto data = GetAchievementsData();
    save.saveData(StorageID::ACHIEVEMENTS, data.getData(), data.getSize());
    SaveToDisk(save, "MySave.save");
}