#include "Asteroids.h"

// For simplicity some global variable
ScreenEmitter ROCK_PARTICLES; // Particle emitter
ScoreCounter SCORE_COUNTER;   // The score counter UI
int SCORE = 0;                // The player score
int ROCK_COUNTER = 0;         // Respawn counter for rocks
int ROCK_SPAWN_DELAY = 80;    // Respawn delay for rocks

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
    // Shows hitboxes of entities with the CollisionC component
    EngineShowHitboxes(true);

    // Shows the shapes of UIObjects
    UIShowHitboxes(true);

    auto& gameScene = SceneGet(GameState::GAME);
    gameScene.addObject(new PlayerBarUI());
    gameScene.addObject(new ScoreCounter());

    auto& gameOverScene = SceneGet(GameState::GAME_OVER);
    gameOverScene.addObject(new GameOverUI());

    // Easy way to set up world bounds - Rect constructor takes a point as the rect size
    CollisionSetWorldBounds(GetScreenDims());

    // Set FPS to 120 - all raylib functions work as usual and are integrated
    SetTargetFPS(120);

    auto loadSound = [](AssetPack& assets) // Load the sounds on a background thread
    {
        assets.forEachIn("SFX", // Iterate the sfx directory for the sounds
                         [&](const Asset& asset)
                         {
                             // Add the sound to the assets
                             ASSETS.sounds[asset.getFileName(false)] = ImportSound(asset);
                         });

        // Add a playlist so it automatically loops
        ASSETS.playlist = ImportPlaylist({assets["Automatav2.mp3"]});
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
                       // Texture dimensions scaled with 3 (3*12) - rotation anchor point is middle per default
                       ComponentGive<CollisionC>(entity, CollisionC::Rectangle(36));
                   });

    // Register the bullet entity
    EntityRegister(EntityType::BULLET, [](entt::entity entity, EntityType type)
                   { ComponentGive<CollisionC>(entity, CollisionC::Rectangle(18)); });

    // Register the house entity
    EntityRegister(EntityType::HOUSE, [](entt::entity entity, EntityType type)
                   { ComponentGive<CollisionC>(entity, CollisionC::Rectangle(45)); });

    // Register the rock entity
    EntityRegister(EntityType::ROCK, [](entt::entity entity, EntityType type)
                   { ComponentGive<CollisionC>(entity, CollisionC::Rectangle(60)); });

    // Register the invisible static camera
    EntityRegister(EntityType::STATIC_CAMERA, [](entt::entity entity, EntityType type) { ComponentGiveCamera(entity); });

    // Create a player
    auto player = EntityCreate(EntityType::PLAYER, {640, 480}, MapID::LEVEL_1);

    // Set the global player
    EngineSetPlayer(player);

    // Create the static camera in the middle of the screen
    EntityCreate(EntityType::STATIC_CAMERA, {GetScreenWidth() / 2.0F, GetScreenHeight() / 2.0F}, MapID::LEVEL_1);

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
}

void Asteroids::onUpdateGame(GameState state)
{
    if (state != GameState::GAME)
        return;
    if (ROCK_COUNTER >= ROCK_SPAWN_DELAY)
    {
        EntityCreate(EntityType::ROCK, {MathRandom(0, 1280), 0}, MapID::LEVEL_1);
        ROCK_COUNTER = 0;
    }
    ROCK_COUNTER++; // Spawn a rock all 80 ticks
}

void Asteroids::onDrawGame(GameState state, Camera2D& camera)
{
    BeginMode2D(camera);
    ClearBackground(BLACK);
    if (state != GameState::GAME)
        return;

    ParticlesDraw(); // Render particles below the entities

    // Get the entities that need to be drawn
    auto drawEntities = EngineGetDrawEntities();
    for (const auto e : drawEntities)
    {
        auto& pos = ComponentGet<PositionC>(e); // Get the implicit position component
        // As the entities don't have sprite sheets we use a simple switch
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
        ComponentGet<PositionC>(self).map = MapID::GAME_OVER_LEVEL;
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

void BulletScript::onCreate(Entity self)
{
    // Play sound on create
    SoundPlay2D(ASSETS.sounds["BULLET_1"], self, 0.5F);
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

void RockScript::onUpdate(entt::entity self, bool updated)
{
    auto& pos = ComponentGet<PositionC>(self);
    pos.rotation += 1;
    pos.pos.y += 1;
}

void RockScript::onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info)
{
    auto& pos = ComponentGet<PositionC>(self).pos;
    auto& col = ComponentGet<CollisionC>(self);
    auto& oPos = ComponentGet<PositionC>(other);

    // Instantiate 100 times
    ParticlesEmit(ROCK_PARTICLES, {pos.x + col.p1 / 2.0F, pos.y + col.p2 / 2.0F}, 100);

    if (oPos.type == EntityType::HOUSE)
    {
        // Uses the player per default when no entity is specified -  (e.g. EngineGetPlayer())
        ComponentGet<PlayerStatsC>().health--; // Lower health when house is hit
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

/// UI

void PlayerBarUI::onDraw(const Rect& bounds)
{
    // Draw the health bar - first outline and then simple rects for each life left
    const PlayerStatsC& stats = ComponentGet<PlayerStatsC>();
    DrawRectangleRec({bounds.x, bounds.y, 152, 25}, ColorAlpha(DARKGRAY, 0.8F));
    DrawText("Player Health", {bounds.x, bounds.y - 20});
    for (int i = 0; i < stats.health; ++i)
    {
        DrawRectangleRec({1 + bounds.x + i * 30, bounds.y + 2, 25, 20}, ColorAlpha(RED, 0.8F));
    }
}

void ScoreCounter::onDraw(const Rect& bounds)
{
    DrawRectangleLinesEx(bounds, 2, WHITE);
    FormatSetValue("Score", SCORE); // Update the format value
    auto text = FormatGetText("Score: ${Score}");
    // Make the text centered inside the element bounds
    DrawTextCentered(EngineGetFont(), text, bounds.mid(), EngineGetFont().baseSize * 3);
}

GameOverUI::GameOverUI() : Button({250, 50}, Anchor::MID_CENTER)
{
    restart.setOnClick(
        [&](MouseButton button)
        {
            auto y = (float)GetScreenHeight() - 45;
            for (int x = 17; x < GetScreenWidth() - 45; x += 50)
            {
                EntityCreate(EntityType::HOUSE, {(float)x, y}, MapID::LEVEL_1);
            }
            auto& stats = ComponentGet<PlayerStatsC>();
            auto& pos = ComponentGet<PositionC>();
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
