#include <magique/magique.hpp>

#include "WizardQuest.h"
#include "ecs/Components.h"
#include "ecs/Systems.h"
#include "loading/Loaders.h"
#include "multiplayer/Multiplayer.h"

void WizardQuest::onStartup(AssetLoader& loader)
{
    // Configure magique
    EngineShowHitboxes(true);

    // Set static world bounds
    CollisionSetWorldBounds({0, 0, 1280, 1000});

    // Register loaders
    loader.registerTask(new EntityLoader(), THREAD_ANY, MEDIUM);
    loader.registerTask(new TileLoader(), THREAD_ANY, MEDIUM);
    loader.registerTask(new TextureLoader(), THREAD_MAIN, MEDIUM);
}

void WizardQuest::onLoadingFinished()
{
    TeleportSystem::setup();
    Multiplayer::init();

    // Set the initial map
    auto map = MapID::LOBBY;

    // Create the player
    EntityCreate(EntityType::PLAYER, {24 * 24}, map);

    // Load the global tileset - the tileset defines the collision (and other) attributes for tiles
    CollisionSetTileset(GLOBAL.tileSet);

    // Adds all solid tiles from the given map as static collision objects - from layer 0 and 1
    CollisionAddTiles(map, GLOBAL.tilemaps[map], {0, 1});
    CollisionAddTiles(map, GLOBAL.tilemaps[MapID::LEVEL_1], {0, 1});

    // Start the game in game state
    EngineSetState(GameState::GAME);
}

void WizardQuest::onDrawGame(GameState state, Camera2D& camera)
{
    BeginMode2D(camera);
    {
        // Get the current map
        const auto& tileMap = GLOBAL.tilemaps[CameraGetMap()];

        // Draw the tilemap using the texture from the tilesheet
        DrawTileMap(tileMap, GLOBAL.tileSheet);

        // Draw the entities using their defined animation data
        for (const auto entity : EngineGetDrawEntities())
        {
            if (EntityHasAll<AnimationC>(entity))
            {
                const auto& pos = ComponentGet<PositionC>(entity);
                const auto& anim = ComponentGet<AnimationC>(entity);
                const auto& mov = ComponentGet<MovementC>(entity);
                anim.drawCurrentFrame(pos, mov.movedLeft);
            }
        }
    }
    EndMode2D();
}

void WizardQuest::onDrawUI(GameState state)
{
    switch (state)
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

void WizardQuest::onUpdateGame(GameState state)
{
    switch (state)
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
void WizardQuest::onUpdateEnd(GameState state) { Multiplayer::postUpdate(); }

// Runs once on shutdown - save our game data
void WizardQuest::onShutDown()
{
    GameStorage save;
    auto data = AchievementExport();
    save.saveString("achievements", data);
    GameStorageToFile(save, "save.save");
}
