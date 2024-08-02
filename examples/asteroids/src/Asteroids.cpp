#include "Asteroids.h"

#include <raylib/raylib.h>

#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/persistence/container/GameSave.h>
#include <magique/core/Core.h>
#include <magique/core/Draw.h>
#include <magique/core/Sound.h>
#include <magique/assets/types/Playlist.h>
#include <magique/ecs/ECS.h>

void Asteroids::onStartup(magique::AssetLoader& al, magique::GameConfig& config)
{
    magique::SetShowHitboxes(true); // Shows red hitboxes

    // Turn off lighting
    magique::SetLightingMode(magique::LightingMode::NONE);

    // Setup screen bounds
    SetWindowSize(1280, 960);

    // Easy way to setup world bounds
    magique::SetStaticWorldBounds({0, 0, 1280, 960});

    // Set FPS to 120 - all raylib functions work as usual and are integrated
    SetTargetFPS(120);

    // Load the sounds on a background thread
    al.registerTask(
        [](magique::AssetContainer& assets)
        {
            // Iterate the sfx directory for the sounds
            assets.iterateDirectory("SFX",
                                    [](const magique::Asset& asset)
                                    {
                                        // Register the sound with the asset manager
                                        auto handle = RegisterSound(asset);
                                        // Register the handle to the sound with its direct filename - without extension
                                        RegisterHandle(handle, asset.getFileName(false));
                                    });

            auto& asset = assets.getAsset("Automatav2.mp3");
            // Create a playlist so it automatically loops
            auto* background = new magique::Playlist{magique::GetMusic(magique::RegisterMusic(asset))};
            magique::PlayPlaylist(background, 0.6F); // Start playlist
        },
        magique::BACKGROUND_THREAD, magique::MEDIUM, 8);

    // Load the texture on the main thread as they require gpu acess
    al.registerTask(
        [](magique::AssetContainer& assets)
        {
            // Iterate the sprites directory for the textures
            assets.iterateDirectory("SPRITES",
                                    [](const magique::Asset& asset)
                                    {
                                        magique::handle handle = RegisterTexture(asset, magique::DEFAULT, 3);
                                        // Register the handle to the texture with its filename - without extension
                                        RegisterHandle(handle, asset.getFileName(false));
                                    });
        },
        magique::MAIN_THREAD, magique::MEDIUM, 2);

    // Set the entity scripts
    SetScript(PLAYER, new PlayerScript());
    SetScript(BULLET, new BulletScript());
    SetScript(ROCK, new RockScript());

    // Register the player entity
    magique::RegisterEntity(EntityID::PLAYER,
                            [](entt::registry& registry, entt::entity entity)
                            {
                                magique::GiveActor(entity);
                                magique::GiveScript(entity);
                                magique::GiveComponent<PlayerStatsC>(entity);
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollisionRect(entity, 36, 36, 18, 18);
                            });

    // Register the bullet entity
    magique::RegisterEntity(EntityID::BULLET,
                            [](entt::registry& registry, entt::entity entity)
                            {
                                magique::GiveScript(entity); // Make it scriptable
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollisionRect(entity,  18, 18, 9, 9);
                            });

    // Register the house entity
    magique::RegisterEntity(EntityID::HOUSE,
                            [](entt::registry& registry, entt::entity entity)
                            {
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollisionRect(entity, 45, 45, 22, 22);
                            });

    // Register the rock entity
    magique::RegisterEntity(EntityID::ROCK,
                            [](entt::registry& registry, entt::entity entity)
                            {
                                magique::GiveScript(entity); // Make it scriptable
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollisionRect(entity, 60, 60, 30, 30);
                            });

    // Register the invisible static camera
    magique::RegisterEntity(EntityID::STATIC_CAMERA,
                            [](entt::registry& registry, entt::entity entity) { magique::GiveCamera(entity); });

    // Create a player
    magique::CreateEntity(PLAYER, 0, 0, MapID::LEVEL_1);

    // Create the static camera in the middle of the screen
    magique::CreateEntity(STATIC_CAMERA, GetScreenWidth() / 2, GetScreenHeight() / 2, MapID::LEVEL_1);

    // Create houses
    auto y = (float)GetScreenHeight() - 45;
    for (int x = 17; x < GetScreenWidth() - 45; x += 50)
    {
        magique::CreateEntity(HOUSE, (float)x, y, MapID::LEVEL_1);
    }
}

void Asteroids::onCloseEvent() { shutDown(); }

void Asteroids::updateGame(entt::registry& registry)
{
    static int ROCK_COUNTER = 80;
    ROCK_COUNTER--;
    if (ROCK_COUNTER == 0)
    {
        auto e = magique::CreateEntity(ROCK, GetRandomValue(0, GetScreenWidth()), 0, MapID::LEVEL_1);
        ROCK_COUNTER = 80;
    }
}

void Asteroids::drawWorld(Camera2D& camera) { ClearBackground(BLACK); }

void Asteroids::drawGame(entt::registry& registry, Camera2D& camera)
{
    // As the entities dont have sprite sheets we use a simple switch
    // Get the entities that need to be drawn
    auto& drawEntities = magique::GetDrawEntities();
    for (const auto e : drawEntities)
    {
        auto& pos = magique::GetComponent<magique::PositionC>(e); // Get the implicit position component
        magique::handle handle;
        switch (pos.type)
        {
        case PLAYER:
            handle = magique::GetHandle(H("PLAYER")); // Used hashed strings
            magique::DrawRegion(GetTexture(handle), pos.x, pos.y);
            break;
        case BULLET:
            handle = magique::GetHandle(H("BULLET"));
            magique::DrawRegion(GetTexture(handle), pos.x, pos.y);
            break;
        case ROCK:
            handle = magique::GetHandle(H("ROCK"));
            magique::DrawRegionEx(GetTexture(handle), pos.x, pos.y, pos.rotation);
            break;
        case HOUSE:
            handle = magique::GetHandle(H("HOUSE"));
            magique::DrawRegion(GetTexture(handle), pos.x, pos.y);
        case STATIC_CAMERA:
            break; // Invisible camera
        }
    }
}

void Asteroids::drawUI(magique::UIRoot& root) {}

// Scripting
void PlayerScript::onKeyEvent(entt::registry& registry, entt::entity self)
{
    auto& pos = magique::GetComponent<magique::PositionC>(self);
    if (IsKeyDown(KEY_W))
        pos.y -= 6;
    if (IsKeyDown(KEY_S))
        pos.y += 6;
    if (IsKeyDown(KEY_A))
        pos.x -= 6;
    if (IsKeyDown(KEY_D))
        pos.x += 6;

    if (IsKeyDown(KEY_SPACE))
    {
        auto& shoot = magique::GetComponent<PlayerStatsC>(self);
        if (shoot.shootCounter == 0)
        {
            magique::CreateEntity(BULLET, pos.x + 3, pos.y - 3, pos.map);
            const auto handle = magique::GetHandle(H("BULLET_1"));
            magique::PlaySound(magique::GetSound(handle), 0.5F);
            shoot.shootCounter = PlayerStatsC::SHOOT_COOLDOWN;
        }
    }
}

void PlayerScript::onTick(entt::registry& registry, entt::entity self)
{
    auto& stats = magique::GetComponent<PlayerStatsC>(self);
    if (stats.shootCounter > 0)
        stats.shootCounter--;
}

void BulletScript::onTick(entt::registry& registry, entt::entity self)
{
    auto& pos = magique::GetComponent<magique::PositionC>(self);
    pos.y -= 8; // Bullets only fly straight up
}

void BulletScript::onStaticCollision(entt::registry& registry, entt::entity self)
{
    magique::DestroyEntity(self);
}

void RockScript::onDynamicCollision(entt::registry& registry, entt::entity self, entt::entity other)
{
    auto& oPos = magique::GetComponent<magique::PositionC>(other);
    if (oPos.type == HOUSE)
    {
        magique::DestroyEntity(other);
    }
    else if (oPos.type == PLAYER)
    {
        auto& stats = magique::GetComponent<PlayerStatsC>(other);
        stats.health--;
    }
    else if (oPos.type == BULLET)
    {
        magique::DestroyEntity(self);
        magique::DestroyEntity(other);
    }
}

void RockScript::onTick(entt::registry& registry, entt::entity self)
{
    auto& pos = magique::GetComponent<magique::PositionC>(self);
    // pos.rotation++;
    pos.y += 1;
}