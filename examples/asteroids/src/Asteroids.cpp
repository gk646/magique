#include "Asteroids.h"

#include <raylib/raylib.h>

#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/core/Core.h>
#include <magique/core/Draw.h>
#include <magique/ecs/ECS.h>

void Asteroids::onStartup(magique::AssetLoader &al, magique::GameConfig &config)
{
    // Turn off lighting
    magique::SetLightingModel(magique::LightingModel::NONE);

    // Setup screen bounds
    SetWindowSize(1280, 960);

    // Set FPS to 120 - all raylib functions work as usual and are integrated
    SetTargetFPS(120);

    // Load the sounds on a background thread
    al.registerTask(
        [](magique::AssetContainer &assets)
        {
            // Iterate the sfx directory for the sounds
            assets.iterateDirectory("SFX",
                                    [](const magique::Asset &asset)
                                    {
                                        // Register the sound with the asset manager
                                        auto handle = RegisterSound(asset);
                                        // Register the handle to the sound with its direct filename - without extension
                                        RegisterHandle(handle, asset.getFileName(false));
                                    });
        },
        magique::BACKGROUND_THREAD, magique::MEDIUM, 8);

    // Load the texture on the main thread as they require gpu acess
    al.registerTask(
        [](magique::AssetContainer &assets)
        {
            // Iterate the sprites directory for the textures
            assets.iterateDirectory("SPRITES",
                                    [](const magique::Asset &asset)
                                    {
                                        magique::handle handle;
                                        // Correctly handle the sprite sheet
                                        if (TextIsEqual(asset.getFileName(), "EXPLOSION SPRITE SHEET.png"))
                                        {
                                            handle = RegisterSpritesheet(asset, 20, 20);
                                        }
                                        else
                                        {
                                            handle = RegisterTexture(asset, magique::DEFAULT, 3);
                                        }
                                        // Register the handle to the texture with its filename - without extension
                                        RegisterHandle(handle, asset.getFileName(false));
                                    });
        },
        magique::MAIN_THREAD, magique::MEDIUM, 2);

    // Set the entity scripts
    SetScript(PLAYER, new PlayerScript());
    SetScript(BULLET, new BulletScript());
    SetScript(HOUSE, new HouseScript());

    // Register the player entity
    magique::RegisterEntity(EntityID::PLAYER,
                            [](entt::registry &registry, entt::entity entity)
                            {
                                magique::GiveActor(entity);
                                magique::GiveScript(entity);
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollision(entity, magique::RECT, 36, 36, 18, 18);
                            });

    // Register the bullet entity
    magique::RegisterEntity(EntityID::BULLET,
                            [](entt::registry &registry, entt::entity entity)
                            {
                                magique::GiveScript(entity); // Make it scriptable
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollision(entity, magique::RECT, 18, 18, 9, 9);
                            });

    // Register the house entity
    magique::RegisterEntity(EntityID::HOUSE,
                            [](entt::registry &registry, entt::entity entity)
                            {
                                magique::GiveScript(entity); // Make it scriptable
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollision(entity, magique::RECT, 45, 45, 22, 22);
                            });

    // Register the invisible static camera
    magique::RegisterEntity(EntityID::STATIC_CAMERA,
                            [](entt::registry &registry, entt::entity entity)
                            {
                                magique::GiveCamera(entity);
                            });

    // Create a player
    magique::CreateEntity(PLAYER, 0, 0, MapID::LEVEL_1);

    // Create the static camera in the middle of the screen
   // magique::CreateEntity(STATIC_CAMERA, GetScreenWidth() / 2, GetScreenHeight() / 2, MapID::LEVEL_1);

    // Create houses
    auto y = (float)GetScreenHeight();
    for (int x = 0; x < GetScreenWidth(); x += 45)
    {
        magique::CreateEntity(HOUSE, (float)x, y, MapID::LEVEL_1);
    }
}

void Asteroids::onCloseEvent() { shutDown(); }

void Asteroids::updateGame(entt::registry &registry) {}

void Asteroids::drawWorld(Camera2D &camera) { ClearBackground(BLACK); }

void Asteroids::drawGame(entt::registry &registry, Camera2D &camera)
{
    // As the entities dont have sprite sheets we use a simple switch

    // Get the entities that need to be drawn
    auto &drawEntities = magique::GetDrawEntities();
    for (const auto e : drawEntities)
    {
        auto &pos = magique::REGISTRY.get<PositionC>(e); // Get the implicit position component
        magique::handle handle;
        switch (pos.type)
        {
        case PLAYER:
            handle = magique::GetHandle(H("PLAYER"));
            magique::DrawRegion(GetTexture(handle), pos.x, pos.y);
            break;
        case BULLET:
            handle = magique::GetHandle(H("BULLET"));
            magique::DrawRegion(GetTexture(handle), pos.x, pos.y);
            break;
        case ROCK:
            break;
        case HOUSE:
            handle = magique::GetHandle(H("HOUSE"));
            magique::DrawRegion(GetTexture(handle), pos.x, pos.y);
        case STATIC_CAMERA:
            break; // Invisible camera
        }
    }
}

void Asteroids::drawUI() {}

// Scripting

void PlayerScript::onKeyEvent(entt::registry &registry, entt::entity self)
{
    auto &pos = registry.get<PositionC>(self);

    if (IsKeyDown(KEY_W))
        pos.y -= 5;
    if (IsKeyDown(KEY_S))
        pos.y += 5;
    if (IsKeyDown(KEY_A))
        pos.x -= 5;
    if (IsKeyDown(KEY_D))
        pos.x += 5;

    if (IsKeyDown(KEY_SPACE))
        magique::CreateEntity(BULLET, pos.x + 3, pos.y - 3, pos.map);
}

void BulletScript::onTick(entt::registry &registry, entt::entity self)
{
    auto &pos = registry.get<PositionC>(self);
    // Bullets only fly straight up
    pos.y -= 8;
}

void HouseScript::onDynamicCollision(entt::registry &registry, entt::entity self, entt::entity other)
{
    auto &oPos = registry.get<PositionC>(other);
    if (oPos.type == ROCK)
    {
        magique::DestroyEntity(self);
    }
}