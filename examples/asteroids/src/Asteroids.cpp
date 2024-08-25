#include "Asteroids.h"

#include <magique/assets/AssetLoader.h>
#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/assets/types/Playlist.h>
#include <magique/core/Core.h>
#include <magique/core/Draw.h>
#include <magique/core/Particles.h>
#include <magique/core/Sound.h>
#include <magique/core/StaticCollision.h>
#include <magique/ecs/ECS.h>
#include <magique/ui/UI.h>

inline magique::ScreenEmitter ROCK_PARTICLES; // For simplicity as global variable
inline entt::entity PLAYER_ID = entt::null;

void Asteroids::onStartup(magique::AssetLoader& loader, magique::GameConfig& config)
{
    magique::SetShowHitboxes(true);
    SetWindowSize(1280, 960); // Setup screen bounds

    magique::SetStaticWorldBounds({0, 0, 1280, 960}); // Easy way to setup world bounds

    SetTargetFPS(120); // Set FPS to 120 - all raylib functions work as usual and are integrated

    auto loadSound = [](magique::AssetContainer& assets) // Load the sounds on a background thread
    {
        assets.iterateDirectory("SFX", // Iterate the sfx directory for the sounds
                                [](const magique::Asset& asset)
                                {
                                    // Register the sound with the asset manager
                                    auto handle = RegisterSound(asset);
                                    // Register the handle to the sound with its direct filename - without extension
                                    RegisterHandle(handle, asset.getFileName(false));
                                });
        auto& asset = assets.getAsset("Automatav2.mp3"); // Create a playlist so it automatically loops

        auto* background = new magique::Playlist{magique::GetMusic(magique::RegisterMusic(asset))};
        magique::PlayPlaylist(background, 0.6F); // Start playlist
    };
    loader.registerTask(loadSound, magique::BACKGROUND_THREAD); // Add the task

    // Load the texture on the main thread as they require gpu acess
    auto loadTextures = [](magique::AssetContainer& assets)
    {
        assets.iterateDirectory("SPRITES", // Iterate the sprites directory for the textures
                                [](const magique::Asset& asset)
                                {
                                    magique::handle handle = RegisterTexture(asset, magique::DEFAULT, 3);
                                    // Register the handle to the texture with its filename - without extension
                                    RegisterHandle(handle, asset.getFileName(false));
                                });
    };
    loader.registerTask(loadTextures, magique::MAIN_THREAD);

    // Set the entity scripts
    SetScript(PLAYER, new PlayerScript());
    SetScript(BULLET, new BulletScript());
    SetScript(ROCK, new RockScript());

    // Register the player entity
    magique::RegisterEntity(EntityID::PLAYER,
                            [](entt::entity entity)
                            {
                                magique::GiveActor(entity);
                                magique::GiveScript(entity);
                                magique::GiveComponent<PlayerStatsC>(entity);
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollisionRect(entity, 36, 36, 18, 18);
                            });

    // Register the bullet entity
    magique::RegisterEntity(EntityID::BULLET,
                            [](entt::entity entity)
                            {
                                magique::GiveScript(entity); // Make it scriptable
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollisionRect(entity, 18, 18, 9, 9);
                            });

    // Register the house entity
    magique::RegisterEntity(EntityID::HOUSE,
                            [](entt::entity entity)
                            {
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollisionRect(entity, 45, 45, 22, 22);
                            });

    // Register the rock entity
    magique::RegisterEntity(EntityID::ROCK,
                            [](entt::entity entity)
                            {
                                magique::GiveScript(entity); // Make it scriptable
                                // Texture dimensions scaled with 3 - and rotate around the middle
                                magique::GiveCollisionRect(entity, 60, 60, 30, 30);
                            });

    // Register the invisible static camera
    magique::RegisterEntity(EntityID::STATIC_CAMERA, [](entt::entity entity) { magique::GiveCamera(entity); });

    PLAYER_ID = magique::CreateEntity(PLAYER, 640, 480, MapID::LEVEL_1); // Create a player

    // Create the static camera in the middle of the screen
    magique::CreateEntity(STATIC_CAMERA, GetScreenWidth() / 2, GetScreenHeight() / 2, MapID::LEVEL_1);

    // Create houses
    auto y = (float)GetScreenHeight() - 45;
    for (int x = 17; x < GetScreenWidth() - 45; x += 50)
    {
        magique::CreateEntity(HOUSE, (float)x, y, MapID::LEVEL_1);
    }

    // Configure emitter - make particles white - spread all around and different lifetime, scale and start velocity
    ROCK_PARTICLES.setColor(WHITE).setLifetime(35, 50).setSpread(360);
    ROCK_PARTICLES.setVelocity(2, 3.5).setScale(0.75, 1.5F);

    magique::SetGameState(GameState::GAME); // Set the initial gamestate
}

void Asteroids::onCloseEvent() { shutDown(); }

void Asteroids::updateGame(GameState gameState)
{
    if (gameState != GameState::GAME)
        return;
    static int ROCK_COUNTER = 80;
    ROCK_COUNTER--;
    if (ROCK_COUNTER == 0)
    {
        magique::CreateEntity(ROCK, GetRandomValue(600, 700), 0, MapID::LEVEL_1);
        ROCK_COUNTER = 80;
    }
}

void Asteroids::drawGame(GameState gameState, Camera2D& camera)
{
    BeginMode2D(camera);
    ClearBackground(BLACK);
    if (gameState != GameState::GAME)
        return;

    magique::DrawParticles(); // Render particles below the entities

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
    magique::DrawHashGridDebug();
    EndMode2D();
}

// UI

void PlayerBarUI::draw(const Rectangle& bounds)
{
    magique::Point anchor = magique::GetUIAnchor(magique::AnchorPosition::TOP_LEFT);
    anchor.y += 50; // Show below the overlay
    auto& stats = magique::GetComponent<PlayerStatsC>(PLAYER_ID);
    DrawRectangleRec({anchor.x, anchor.y, 152, 25}, ColorAlpha(DARKGRAY, 0.8F));
    DrawText("Player Health", anchor.x, anchor.y - 20, 20, WHITE);
    for (int i = 0; i < stats.health; ++i)
    {
        DrawRectangleRec({1 + anchor.x + i * 30, anchor.y + 2, 25, 20}, ColorAlpha(RED, 0.8F));
    }
}

void GameOverUI::onClick(const Rectangle& bounds)
{
    // Spawn new houses
    auto y = (float)GetScreenHeight() - 45;
    for (int x = 17; x < GetScreenWidth() - 45; x += 50)
    {
        magique::CreateEntity(HOUSE, (float)x, y, MapID::LEVEL_1);
    }
    auto& stats = magique::GetComponent<PlayerStatsC>(PLAYER_ID);
    auto& pos = magique::GetComponent<magique::PositionC>(PLAYER_ID);
    stats.health = 5;
    pos.x = 640; // Reset player position
    pos.y = 480;
    pos.map = MapID::LEVEL_1;
    magique::SetGameState(GameState::GAME);
}

// Scripting

void PlayerScript::onKeyEvent(entt::entity self)
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

void PlayerScript::onTick(entt::entity self)
{
    auto& stats = magique::GetComponent<PlayerStatsC>(self);
    if (stats.shootCounter > 0)
        stats.shootCounter--;
    if (stats.health <= 0)
    {
        magique::SetGameState(GameState::GAME_OVER);
        // Move player (actor) to different map to avoid updating all entities and destroy all current ones
        auto& pos = magique::GetComponent<magique::PositionC>(self);
        pos.map = MapID::GAME_OVER_LEVEL;
        magique::ClearEntityCache();
        magique::DestroyAllEntities({ROCK, HOUSE}); // Destroy all rocks + houses
    }
}

void BulletScript::onTick(entt::entity self)
{
    auto& pos = magique::GetComponent<magique::PositionC>(self);
    pos.y -= 8; // Bullets only fly straight up
}

void BulletScript::onStaticCollision(entt::entity self, const magique::CollisionInfo& info, magique::ColliderInfo cInfo)
{
    magique::DestroyEntity(self);
}

void RockScript::onDynamicCollision(entt::entity self, entt::entity other, const magique::CollisionInfo& info)
{
    if (!magique::EntityExists(self)) // If multiple collisions happen and rock was already destroyed
        return;
    auto& pos = magique::GetComponent<magique::PositionC>(self);
    auto& col = magique::GetComponent<magique::CollisionC>(self);
    auto& oPos = magique::GetComponent<magique::PositionC>(other);

    ROCK_PARTICLES.setEmissionPosition(pos.x + col.p1 / 2.0F, pos.y + col.p2 / 2.0F); // Set to emit from the center
    magique::CreateScreenParticle(ROCK_PARTICLES, 100);                               // Instantiate 100 times
    if (oPos.type == HOUSE)
    {
        magique::GetComponent<PlayerStatsC>(PLAYER_ID).health--; // Lower health when house is hit
        magique::DestroyEntity(other);
    }
    else if (oPos.type == PLAYER)
    {
        auto& stats = magique::GetComponent<PlayerStatsC>(other);
        stats.health--;
    }
    else if (oPos.type == BULLET)
    {
        magique::DestroyEntity(other);
    }
    magique::DestroyEntity(self);
}

void RockScript::onTick(entt::entity self)
{
    auto& pos = magique::GetComponent<magique::PositionC>(self);
    // pos.rotation++;
    pos.y += 1;
}