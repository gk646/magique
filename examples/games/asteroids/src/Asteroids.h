#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <magique/core/Game.h>
#include <magique/ecs/Scripting.h>
#include <magique/ui/UIObject.h>
#include <magique/ui/controls/Button.h>

using namespace magique;
// Its used explicitly each time to denote magique functions
// It is advised and safe to use: "using namespace magique;"

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

// The game class
struct Asteroids final : Game
{
    Asteroids() : Game("Asteroids") {}
    void onStartup(AssetLoader& loader, GameConfig& config) override;
    void onCloseEvent() override;
    void updateGame(GameState gameState) override;
    void drawGame(GameState gameState, Camera2D& camera) override;
};

struct PlayerScript final : EntityScript
{
    void onKeyEvent(entt::entity self) override;
    void onTick(entt::entity self) override;
};

struct BulletScript final : EntityScript
{
    void onTick(entt::entity self) override;
    void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info) override;
};

struct RockScript final : EntityScript
{
    void onTick(entt::entity self) override;
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override;
};

struct PlayerBarUI final : UIObject
{
    PlayerBarUI() : UIObject(50, 50, 200, 50) {}
    void draw(const Rectangle& bounds) override;
};

struct GameOverUI final : Button
{
    GameOverUI() : Button(960, 520, 150, 50) {}
    void onClick(const Rectangle& bounds) override;
};

#endif // ASTEROIDS_H