#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <magique/magique.hpp> // Single header include

using namespace magique;

// Entity identifiers
enum class EntityType : uint16_t
{
    PLAYER,
    BULLET,
    ROCK,
    HOUSE,
    STATIC_CAMERA, // In this example the camera is static and not attached to a entity
};

enum class GameState : uint8_t
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
    void onUpdateGame(GameState gameState) override;
    void onDrawGame(GameState gameState, Camera2D& camera) override;
    void onDrawUI(GameState gameState) override;
};

/// Scripts

struct PlayerScript final : EntityScript
{
    void onUpdate(entt::entity self, bool updated) override;
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override;
};

struct BulletScript final : EntityScript
{
    void onUpdate(entt::entity self, bool updated) override;
    void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info) override;
};

struct RockScript final : EntityScript
{
    void onUpdate(entt::entity self, bool updated) override;
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
    PlayerBarUI() : UIObject({50, 70, 200, 50}) {}
    void onDraw(const Rect& bounds) override;
};

struct ScoreCounter final : UIObject
{
    ScoreCounter() : UIObject({200, 50}, Anchor::TOP_CENTER) {}
    void onDraw(const Rect& bounds) override;
    TextButton score{""};
};

struct GameOverUI final : Button
{
    GameOverUI();
    void onDraw(const Rect& bounds) override;

    TextButton restart{"Restart"};
};

#endif // ASTEROIDS_H
