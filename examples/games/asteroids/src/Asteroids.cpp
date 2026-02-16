#include "Asteroids.h"

using namespace magique;

// For simplicity some global variable
ScreenEmitter ROCK_PARTICLES;        // Particle emitter
entt::entity PLAYER_ID = entt::null; // Saving the player id
PlayerBarUI PLAYER_BAR;              // The player UI
ScoreCounter SCORE_COUNTER;          // The score counter UI
GameOverUI GAMEOVER;                 // The game over UI
int SCORE = 0;                       // The player score
int ROCK_COUNTER = 0;                // Respawn counter for rocks
int ROCK_SPAWN_DELAY = 80;           // Respawn delay for rocks
float FONT_SIZE = 30;                // Text size
/// Game

void Asteroids::onStartup(AssetLoader& loader)
{
    //SetShowHitboxes(true); // Enable if wanted

    SetStaticWorldBounds({0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}); // Easy way to set up world bounds

    SetTargetFPS(120); // Set FPS to 120 - all raylib functions work as usual and are integrated

    auto loadSound = [](AssetContainer& assets) // Load the sounds on a background thread
    {
        assets.iterateDirectory("SFX", // Iterate the sfx directory for the sounds
                                [](const Asset& asset)
                                {
                                    // Register the sound with the asset manager
                                    auto handle = RegisterSound(asset);
                                    // Register the handle to the sound with its direct filename - without extension
                                    RegisterHandle(handle, asset.getFileName(false));
                                });
        auto& asset = assets.getAsset("Automatav2.mp3"); // Create a playlist so it automatically loops

        auto& playList = GetPlaylist(RegisterPlaylist({asset}));
        PlayPlaylist(playList, 0.6F); // Start playlist
    };
    loader.registerTask(loadSound, THREAD_ANY); // Add the task

    // Load the texture on the main thread as they require gpu access
    auto loadTextures = [](AssetContainer& assets)
    {
        assets.iterateDirectory("SPRITES", // Iterate the sprites directory for the textures
                                [](const Asset& asset)
                                {
                                    handle handle = RegisterTexture(asset, {}, 3);
                                    // Register the handle to the texture with its filename - without extension
                                    RegisterHandle(handle, asset.getFileName(false));
                                });
    };
    loader.registerTask(loadTextures, THREAD_MAIN);

    // Set the entity scripts
    SetEntityScript(PLAYER, new PlayerScript());
    SetEntityScript(BULLET, new BulletScript());
    SetEntityScript(ROCK, new RockScript());
    SetEntityScript(HOUSE, new HouseScript());

    // Register the player entity
    RegisterEntity(PLAYER,
                   [](entt::entity entity, EntityType type)
                   {
                       GiveActor(entity);
                       GiveComponent<PlayerStatsC>(entity);
                       // Texture dimensions scaled with 3 - and rotate around the middle
                       GiveCollisionRect(entity, 36, 36, 18, 18);
                   });

    // Register the bullet entity
    RegisterEntity(BULLET,
                   [](entt::entity entity, EntityType type)
                   {
                       // Texture dimensions scaled with 3 - and rotate around the middle
                       GiveCollisionRect(entity, 18, 18, 9, 9);
                   });

    // Register the house entity
    RegisterEntity(HOUSE,
                   [](entt::entity entity, EntityType type)
                   {
                       // Texture dimensions scaled with 3 - and rotate around the middle
                       GiveCollisionRect(entity, 45, 45, 22, 22);
                   });

    // Register the rock entity
    RegisterEntity(ROCK,
                   [](entt::entity entity, EntityType type)
                   {
                       // Texture dimensions scaled with 3 - and rotate around the middle
                       GiveCollisionRect(entity, 60, 60, 30, 30);
                   });

    // Register the invisible static camera
    RegisterEntity(STATIC_CAMERA,
                   [](entt::entity entity, EntityType type)
                   {
                       SetIsEntityScripted(entity, false); // Make it non scripted
                       GiveCamera(entity);
                   });

    PLAYER_ID = CreateEntity(PLAYER, 640, 480, MapID::LEVEL_1); // Create a player

    // Create the static camera in the middle of the screen
    auto entity = CreateEntity(STATIC_CAMERA, GetScreenWidth() / 2, GetScreenHeight() / 2, MapID::LEVEL_1);
    SetIsEntityScripted(entity, false); // Camera is not a scripted entity

    // Create houses
    auto y = (float)GetScreenHeight() - 45;
    for (int x = 17; x < GetScreenWidth() - 45; x += 50)
    {
        CreateEntity(HOUSE, (float)x, y, MapID::LEVEL_1);
    }

    // Configure emitter - make particles white - spread all around and different lifetime, scale and start velocity
    ROCK_PARTICLES.setColor(WHITE).setLifetime(35, 50).setSpread(360);
    ROCK_PARTICLES.setVelocity(2, 3.5).setScale(0.75, 1.5F);

    SetGameState(GameState::GAME); // Set the initial gamestate
    // As we use a static camera with no collision component we offset by the player half the player dimensions
    // So the camera is centered on the player middle and no the top left
    SetCameraPositionOffset(18, 18);
}

void Asteroids::updateGame(GameState gameState)
{
    if (gameState != GameState::GAME)
        return;
    if (ROCK_COUNTER >= ROCK_SPAWN_DELAY)
    {
        CreateEntity(ROCK, GetRandomValue(0, 1280), 0, MapID::LEVEL_1);
        ROCK_COUNTER = 0;
    }
    ROCK_COUNTER++; // Spawn a rock all 80 ticks
}

void Asteroids::drawGame(GameState gameState, Camera2D& camera)
{
    BeginMode2D(camera);
    ClearBackground(BLACK);
    if (gameState != GameState::GAME)
        return;

    DrawParticles(); // Render particles below the entities

    // As the entities don't have sprite sheets we use a simple switch
    auto& drawEntities = GetDrawEntities(); // Get the entities that need to be drawn
    for (const auto e : drawEntities)
    {
        auto& pos = ComponentGet<PositionC>(e); // Get the implicit position component
        handle handle;
        switch (pos.type)
        {
        case PLAYER:
            handle = GetHandle(GetHash("PLAYER")); // Used hashed strings
            DrawRegion(GetTexture(handle), pos.x, pos.y);
            break;
        case BULLET:
            handle = GetHandle(GetHash("BULLET"));
            DrawRegion(GetTexture(handle), pos.x, pos.y);
            break;
        case ROCK:
            {
                handle = GetHandle(GetHash("ROCK"));
                const auto texture = GetTexture(handle);
                Point dims = {(float)texture.width, (float)texture.height}; // Get the texture dimensions
                // Draw the texture rotated around its middle point
                DrawRegionPro(texture, {pos.x, pos.y, dims.x, dims.y}, pos.rotation, dims / 2);
            }
            break;
        case HOUSE:
            handle = GetHandle(GetHash("HOUSE"));
            DrawRegion(GetTexture(handle), pos.x, pos.y);
        case STATIC_CAMERA:
            break; // Invisible camera
        }
    }
    EndMode2D();
}

void Asteroids::drawUI(GameState gameState)
{
    PLAYER_BAR.draw();
    SCORE_COUNTER.draw();
    if (gameState == GameState::GAME_OVER)
        GAMEOVER.draw();
}

/// Scripting

void PlayerScript::onKeyEvent(entt::entity self)
{
    auto& pos = ComponentGet<PositionC>(self);
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
        auto& shoot = ComponentGet<PlayerStatsC>(self);
        if (shoot.shootCounter == 0)
        {
            CreateEntity(BULLET, pos.x + 3, pos.y - 3, pos.map);
            PlaySound(GetSound(GetHash("BULLET_1")), 0.5F);
            shoot.shootCounter = PlayerStatsC::SHOOT_COOLDOWN;
        }
    }
}

void PlayerScript::onTick(entt::entity self, bool updated)
{
    auto& stats = ComponentGet<PlayerStatsC>(self);
    if (stats.shootCounter > 0)
        stats.shootCounter--;
    if (stats.health <= 0)
    {
        SetGameState(GameState::GAME_OVER);
        // Move player (actor) to different map to avoid updating all entities and destroy all current ones
        auto& pos = ComponentGet<PositionC>(self);
        pos.map = MapID::GAME_OVER_LEVEL;
        DestroyEntities({ROCK, HOUSE}); // Destroy all rocks + houses
        ClearEntityCache();
    }
}

void PlayerScript::onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info)
{
    const auto& oPos = ComponentGet<PositionC>(other);
    if (oPos.type != BULLET && oPos.type != HOUSE)
        AccumulateCollision(info);
}

void BulletScript::onTick(entt::entity self, bool updated)
{
    auto& pos = ComponentGet<PositionC>(self);
    pos.y -= 8; // Bullets only fly straight up
}

void BulletScript::onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info)
{
    DestroyEntity(self); // Destroy on static collision
}

void RockScript::onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info)
{
    if (!EntityExists(other))
        return;
    auto& pos = ComponentGet<PositionC>(self);
    auto& col = ComponentGet<CollisionC>(self);
    auto& oPos = ComponentGet<PositionC>(other);

    ROCK_PARTICLES.setEmissionPosition(pos.x + col.p1 / 2.0F, pos.y + col.p2 / 2.0F); // Set to emit from the center
    ParticlesEmit(ROCK_PARTICLES, 100);                                        // Instantiate 100 times
    if (oPos.type == HOUSE)
    {
        ComponentGet<PlayerStatsC>(PLAYER_ID).health--; // Lower health when house is hit
        DestroyEntity(other);
    }
    else if (oPos.type == PLAYER)
    {
        auto& stats = ComponentGet<PlayerStatsC>(other);
        stats.health--;
    }
    else if (oPos.type == BULLET)
    {
        DestroyEntity(other);
        SCORE++;
    }
    DestroyEntity(self);
}

void RockScript::onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info)
{
    DestroyEntity(self); // Destroy rock on static collision
}

void RockScript::onTick(entt::entity self, bool updated)
{
    auto& pos = ComponentGet<PositionC>(self);
    pos.rotation++;
    pos.y += 1;
}

/// UI

void PlayerBarUI::onDraw(const Rectangle& bounds)
{
    // Draw the health bar - first outline and then simple rects for each life left
    const PlayerStatsC& stats = ComponentGet<PlayerStatsC>(PLAYER_ID);
    DrawRectangleRec({bounds.x, bounds.y, 152, 25}, ColorAlpha(DARKGRAY, 0.8F));
    DrawText("Player Health", bounds.x, bounds.y - 20, 20, WHITE);
    for (int i = 0; i < stats.health; ++i)
    {
        DrawRectangleRec({1 + bounds.x + i * 30, bounds.y + 2, 25, 20}, ColorAlpha(RED, 0.8F));
    }
}

void ScoreCounter::onDraw(const Rectangle& bounds)
{
    DrawRectangleLinesEx(bounds, 2, WHITE);
    SetFormatValue("Score", SCORE); // Update the format value
    const char* text = FormatGetText("Score: ${Score}");
    const auto width = MeasureText(text, FONT_SIZE);
    auto middlePos = GetCenteredPos(bounds, width, FONT_SIZE); // Make the text centered inside the element bounds
    DrawText(text, middlePos.x, middlePos.y, FONT_SIZE, WHITE);
}

void GameOverUI::onDraw(const Rectangle& bounds)
{
    drawDefault(bounds);
    const char* gameOver = "Game Over";
    const Point topCenter = GetUIAnchor(Anchor::TOP_CENTER, MeasureText(gameOver, FONT_SIZE), 50);
    DrawText("Game Over", topCenter.x, topCenter.y, FONT_SIZE, WHITE);
    const char* restart = "Restart";
    // Make the text centered inside the element bounds
    auto middlePos = GetCenteredPos(bounds, MeasureText(restart, FONT_SIZE), FONT_SIZE);
    DrawText(restart, middlePos.x, middlePos.y, FONT_SIZE, WHITE);
}

void GameOverUI::onClick(const Rectangle& bounds, int button)
{
    // Spawn new houses
    auto y = (float)GetScreenHeight() - 45;
    for (int x = 17; x < GetScreenWidth() - 45; x += 50)
    {
        CreateEntity(HOUSE, (float)x, y, MapID::LEVEL_1);
    }
    auto& stats = ComponentGet<PlayerStatsC>(PLAYER_ID);
    auto& pos = ComponentGet<PositionC>(PLAYER_ID);
    stats.health = 5;
    pos.x = 640; // Reset player position
    pos.y = 480;
    pos.map = MapID::LEVEL_1;
    SCORE = 0;
    SetGameState(GameState::GAME);
}