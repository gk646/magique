#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <magique/core/Game.h>
#include <magique/ecs/Scripting.h>
#include <magique/ui/types/UIObject.h>
#include <magique/ui/controls/Button.h>

// NOTE: the namespace magique:: can be avoided by using: "using namespace magique;"
// Its used explicitly each time to denote magique functions
// It is advised and safe to use: "using namespace magique;"

// Entity identifiers
enum EntityID : uint16_t
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
struct Asteroids final : magique::Game
{
    Asteroids() : Game("Asteroids") {}
    void onStartup(magique::AssetLoader& loader, magique::GameConfig& config) override;
    void onCloseEvent() override;
    void updateGame(GameState gameState) override;
    void drawGame(GameState gameState) override;
};

struct PlayerScript final : magique::EntityScript
{
    void onKeyEvent(entt::registry& registry, entt::entity self) override;
    void onTick(entt::registry& registry, entt::entity self) override;
};

struct BulletScript final : magique::EntityScript
{
    void onTick(entt::registry& registry, entt::entity self) override;
    void onStaticCollision(entt::registry& registry, entt::entity self) override;
};

struct RockScript final : magique::EntityScript
{
    void onTick(entt::registry& registry, entt::entity self) override;
    void onDynamicCollision(entt::registry& registry, entt::entity self, entt::entity other) override;
};

struct PlayerBarUI final : magique::UIObject
{
    PlayerBarUI() = default;
    void draw() override;
};

struct GameOverUI final : magique::Button
{
    GameOverUI() : Button(GameState::GAME, 960, 520, 150, 50) {}
    void onClick(const Rectangle &bounds) override;
};

#endif // ASTEROIDS_H