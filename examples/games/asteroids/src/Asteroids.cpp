#include "Asteroids.h"

#include "magique/ui/Scene.h"

using namespace magique;

// For simplicity some global variable
ScreenEmitter ROCK_PARTICLES;        // Particle emitter
entt::entity PLAYER_ID = entt::null; // Saving the player id
ScoreCounter SCORE_COUNTER;          // The score counter UI
int SCORE = 0;                       // The player score
int ROCK_COUNTER = 0;                // Respawn counter for rocks
int ROCK_SPAWN_DELAY = 80;           // Respawn delay for rocks

struct GameAssets
{
    // Using strings is quite slow - either use a enum or the compiled time hashes strings
    StringHashMap<Sound> sounds;
    StringHashMap<TextureRegion> textures;
    Playlist playlist;
};

GameAssets ASSETS{};

/// Game

void Asteroids::onStartup(AssetLoader& loader)
{
    EngineShowHitboxes(true); // Enable if wanted
    UIShowHitboxes(true);

    auto& gameScene = SceneGet(GameState::GAME);
    gameScene.addObject(new PlayerBarUI());
    gameScene.addObject(new ScoreCounter());

    auto& gameOverScene = SceneGet(GameState::GAME_OVER);
    gameOverScene.addObject(new GameOverUI());

    CollisionSetWorldBounds(
        {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}); // Easy way to set up world bounds

    SetTargetFPS(120); // Set FPS to 120 - all raylib functions work as usual and are integrated

    auto loadSound = [](AssetPack& assets) // Load the sounds on a background thread
    {
        assets.forEachIn("SFX", // Iterate the sfx directory for the sounds
                         [&](const Asset& asset)
                         {
                             // Add the sound to the assets
                             ASSETS.sounds[asset.getFileName(false)] = ImportSound(asset);
                         });

        // Add a playlist so it automatically loops
        ASSETS.playlist = ImportPlaylist({assets.getAsset("Automatav2.mp3")});
        PlaylistPlay(ASSETS.playlist, 0.6F); // Start playlist
    };
    loader.registerTask(loadSound, THREAD_ANY); // Add the task

    // Load the texture on the main thread as they require gpu access
    auto loadTextures = [](AssetPack& assets)
    {
        assets.forEachIn("SPRITES", // Iterate the sprites directory for the textures
                         [](const Asset& asset)
                         {
                             // Adds the texture with its filename - without extension
                             ASSETS.textures[asset.getFileName(false)] = ImportTexture(asset, {}, 3);
                         });
    };
    loader.registerTask(loadTextures, THREAD_MAIN);

    // Set the entity scripts
    ScriptingSetScript(EntityType::PLAYER, new PlayerScript());
    ScriptingSetScript(EntityType::BULLET, new BulletScript());
    ScriptingSetScript(EntityType::ROCK, new RockScript());
    ScriptingSetScript(EntityType::HOUSE, new HouseScript());

    // Register the player entity
    EntityRegister(EntityType::PLAYER,
                   [](entt::entity entity, EntityType type)
                   {
                       ComponentGiveActor(entity);
                       ComponentGive<PlayerStatsC>(entity);
                       // Texture dimensions scaled with 3 - and rotate around the middle
                       ComponentGiveCollisionRect(entity, {36, 36});
                   });

    // Register the bullet entity
    EntityRegister(EntityType::BULLET,
                   [](entt::entity entity, EntityType type)
                   {
                       // Texture dimensions scaled with 3 - and rotate around the middle
                       ComponentGiveCollisionRect(entity, {18, 18});
                   });

    // Register the house entity
    EntityRegister(EntityType::HOUSE,
                   [](entt::entity entity, EntityType type)
                   {
                       // Texture dimensions scaled with 3 - and rotate around the middle
                       ComponentGiveCollisionRect(entity, {45, 45});
                   });

    // Register the rock entity
    EntityRegister(EntityType::ROCK,
                   [](entt::entity entity, EntityType type)
                   {
                       // Texture dimensions scaled with 3 - and rotate around the middle
                       ComponentGiveCollisionRect(entity, {60, 60});
                   });

    // Register the invisible static camera
    EntityRegister(EntityType::STATIC_CAMERA,
                   [](entt::entity entity, EntityType type)
                   {
                       ScriptingSetScripted(entity, false); // Make it non scripted
                       ComponentGiveCamera(entity);
                   });

    PLAYER_ID = EntityCreate(EntityType::PLAYER, {640, 480}, MapID::LEVEL_1); // Create a player

    // Create the static camera in the middle of the screen
    auto entity =
        EntityCreate(EntityType::STATIC_CAMERA, {GetScreenWidth() / 2.0F, GetScreenHeight() / 2.0F}, MapID::LEVEL_1);
    ScriptingSetScripted(entity, false); // Camera is not a scripted entity

    // Create houses
    auto y = (float)GetScreenHeight() - 45;
    for (int x = 17; x < GetScreenWidth() - 45; x += 50)
    {
        EntityCreate(EntityType::HOUSE, {(float)x, y}, MapID::LEVEL_1);
    }

    // Configure emitter - make particles white - spread all around and different lifetime, scale and start velocity
    ROCK_PARTICLES.setColors({WHITE}).setLifetime({0.2, 0.6}).setSpread(360);
    ROCK_PARTICLES.setVelocityRange({2, 3.5}).setScale({0.75, 1.5F});

    EngineSetState(GameState::GAME); // Set the initial gamestate

    // As we use a static camera with no collision component we offset by the player half the player dimensions
    // So the camera is centered on the player middle and no the top left
    CameraSetTargetOffset(18, 18);
}

void Asteroids::onUpdateGame(GameState gameState)
{
    if (gameState != GameState::GAME)
        return;
    if (ROCK_COUNTER >= ROCK_SPAWN_DELAY)
    {
        EntityCreate(EntityType::ROCK, {MathRandom(0, 1280), 0}, MapID::LEVEL_1);
        ROCK_COUNTER = 0;
    }
    ROCK_COUNTER++; // Spawn a rock all 80 ticks
}

void Asteroids::onDrawGame(GameState gameState, Camera2D& camera)
{
    BeginMode2D(camera);
    ClearBackground(BLACK);
    if (gameState != GameState::GAME)
        return;

    ParticlesDraw(); // Render particles below the entities

    // As the entities don't have sprite sheets we use a simple switch
    auto& drawEntities = EngineGetDrawEntities(); // Get the entities that need to be drawn
    for (const auto e : drawEntities)
    {
        auto& pos = ComponentGet<PositionC>(e); // Get the implicit position component
        switch (pos.type)
        {
        case EntityType::PLAYER:
            DrawRegion(ASSETS.textures["PLAYER"], pos.pos);
            break;
        case EntityType::BULLET:
            DrawRegion(ASSETS.textures["BULLET"], pos.pos);
            break;
        case EntityType::ROCK:
            {
                const auto& texture = ASSETS.textures["ROCK"];
                // Draw the texture rotated around its middle point
                DrawRegionPro(texture, {pos.pos, texture.getSize()}, pos.rotation, texture.getSize() / 2);
            }
            break;
        case EntityType::HOUSE:
            DrawRegion(ASSETS.textures["HOUSE"], pos.pos);
        case EntityType::STATIC_CAMERA:
            break; // Invisible camera
        }
    }
    EndMode2D();
}

void Asteroids::onDrawUI(GameState gameState)
{
    auto& current = SceneGet(gameState);
    current.draw();
}

/// Scripting

void PlayerScript::onUpdate(entt::entity self, bool updated)
{
    auto& pos = ComponentGet<PositionC>(self);
    if (IsKeyDown(KEY_W))
        pos.pos.y -= 6;
    if (IsKeyDown(KEY_S))
        pos.pos.y += 6;
    if (IsKeyDown(KEY_A))
        pos.pos.x -= 6;
    if (IsKeyDown(KEY_D))
        pos.pos.x += 6;

    if (IsKeyDown(KEY_SPACE))
    {
        auto& shoot = ComponentGet<PlayerStatsC>(self);
        if (shoot.shootCounter == 0)
        {
            EntityCreate(EntityType::BULLET, {pos.pos.x + 3, pos.pos.y - 3}, pos.map);
            SoundPlay2D(ASSETS.sounds["BULLET_1"], self, 0.5F);
            shoot.shootCounter = PlayerStatsC::SHOOT_COOLDOWN;
        }
    }

    auto& stats = ComponentGet<PlayerStatsC>(self);
    if (stats.shootCounter > 0)
        stats.shootCounter--;
    if (stats.health <= 0)
    {
        EngineSetState(GameState::GAME_OVER);
        // Move player (actor) to different map to avoid updating all entities and destroy all current ones
        auto& pos = ComponentGet<PositionC>(self);
        pos.map = MapID::GAME_OVER_LEVEL;
        EntityDestroy({EntityType::ROCK, EntityType::HOUSE}); // Destroy all rocks + houses
        EngineClearCache();
    }
}

void PlayerScript::onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info)
{
    const auto& oPos = ComponentGet<PositionC>(other);
    if (oPos.type != EntityType::BULLET && oPos.type != EntityType::HOUSE)
        AccumulateCollision(info);
}

void BulletScript::onUpdate(entt::entity self, bool updated)
{
    auto& pos = ComponentGet<PositionC>(self).pos;
    pos.y -= 8; // Bullets only fly straight up
}

void BulletScript::onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info)
{
    EntityDestroy(self); // Destroy on static collision
}

void RockScript::onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info)
{
    if (!EntityExists(other))
        return;
    auto& pos = ComponentGet<PositionC>(self).pos;
    auto& col = ComponentGet<CollisionC>(self);
    auto& oPos = ComponentGet<PositionC>(other);

    // Instantiate 100 times
    ParticlesEmit(ROCK_PARTICLES, {pos.x + col.p1 / 2.0F, pos.y + col.p2 / 2.0F}, 100);

    if (oPos.type == EntityType::HOUSE)
    {
        ComponentGet<PlayerStatsC>(PLAYER_ID).health--; // Lower health when house is hit
        EntityDestroy(other);
    }
    else if (oPos.type == EntityType::PLAYER)
    {
        auto& stats = ComponentGet<PlayerStatsC>(other);
        stats.health--;
    }
    else if (oPos.type == EntityType::BULLET)
    {
        EntityDestroy(other);
        SCORE++;
    }
    EntityDestroy(self);
}

void RockScript::onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info)
{
    EntityDestroy(self); // Destroy rock on static collision
}

void RockScript::onUpdate(entt::entity self, bool updated)
{
    auto& pos = ComponentGet<PositionC>(self);
    pos.rotation += 1;
    pos.pos.y += 1;
}

/// UI

void PlayerBarUI::onDraw(const Rect& bounds)
{
    // Draw the health bar - first outline and then simple rects for each life left
    const PlayerStatsC& stats = ComponentGet<PlayerStatsC>(PLAYER_ID);
    DrawRectangleRec({bounds.x, bounds.y, 152, 25}, ColorAlpha(DARKGRAY, 0.8F));
    DrawPixelText(EngineGetFont(),"Player Health", {bounds.x, bounds.y - 20},3);
    for (int i = 0; i < stats.health; ++i)
    {
        DrawRectangleRec({1 + bounds.x + i * 30, bounds.y + 2, 25, 20}, ColorAlpha(RED, 0.8F));
    }
}

void ScoreCounter::onDraw(const Rect& bounds)
{
    DrawRectangleLinesEx(bounds, 2, WHITE);
    FormatSetValue("Score", SCORE); // Update the format value
    const char* text = FormatGetText("Score: ${Score}");
    // Make the text centered inside the element bounds
    DrawTextCenteredRect(EngineGetFont(), text,EngineGetFont().baseSize*3, bounds);
}

GameOverUI::GameOverUI() : Button({250, 50}, Anchor::MID_CENTER)
{
    restart.setOnClick(
        [&](const Rect& bounds, int button)
        {
            auto y = (float)GetScreenHeight() - 45;
            for (int x = 17; x < GetScreenWidth() - 45; x += 50)
            {
                EntityCreate(EntityType::HOUSE, {(float)x, y}, MapID::LEVEL_1);
            }
            auto& stats = ComponentGet<PlayerStatsC>(PLAYER_ID);
            auto& pos = ComponentGet<PositionC>(PLAYER_ID);
            stats.health = 5;
            pos.pos = {640, 480}; // Reset player position
            pos.map = MapID::LEVEL_1;
            SCORE = 0;
            EngineSetState(GameState::GAME);
        });
}

void GameOverUI::onDraw(const Rect& bounds)
{
    restart.align(Anchor::MID_CENTER, *this);
    restart.draw();
}
