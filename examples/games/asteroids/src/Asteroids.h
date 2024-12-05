#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <magique/magique.hpp> // Single header include

using namespace magique;

// Entity identifiers
enum EntityType : uint16_t
{
    PLAYER,
    BULLET,
    ROCK,
    HOUSE,
    STATIC_CAMERA, // In this example the camera is static and not attached to a entity
};

enum class GameState
{
    GAME,
    GAME_OVER,
};

enum class MapID : uint8_t
{
    LEVEL_1,
    GAME_OVER_LEVEL,
};

struct PlayerStatsC final
{
    static constexpr int SHOOT_COOLDOWN = 35;
    static constexpr int HIT_COOLDOWN = 35;
    int shootCounter = 0;
    int hitCounter = 0;
    int health = 5;
};

/// Game class

struct Asteroids final : Game
{
    Asteroids() : Game("magique - Asteroids") {}
    void onStartup(AssetLoader& loader) override;
    void updateGame(GameState gameState) override;
    void drawGame(GameState gameState, Camera2D& camera) override;
    void drawUI(GameState gameState) override;
};

/// Scripts

struct PlayerScript final : EntityScript
{
    void onKeyEvent(entt::entity self) override;
    void onTick(entt::entity self, bool updated) override;
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override;
};

struct BulletScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override;
    void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info) override;
};

struct RockScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override;
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override;
    void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info) override;
};

struct HouseScript final : EntityScript
{
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override
    {
        // Make it not react to collision
    }
};

/// UI

struct PlayerBarUI final : UIObject
{
    PlayerBarUI() : UIObject(50, 70, 200, 50) {}
    void onDraw(const Rectangle& bounds) override;
};

struct ScoreCounter final : UIObject
{
    ScoreCounter() : UIObject(200, 50, Anchor::TOP_CENTER) {}
    void onDraw(const Rectangle& bounds) override;
};

struct GameOverUI final : Button
{
    GameOverUI() : Button(250,50, Anchor::MID_CENTER) {}
    void onClick(const Rectangle& bounds, int button) override;
    void onDraw(const Rectangle &bounds) override;
};

#endif // ASTEROIDS_H