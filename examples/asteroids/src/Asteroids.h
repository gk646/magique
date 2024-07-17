#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <magique/core/Game.h>
#include <magique/ecs/InternalScripting.h>

// Entity identifiers
enum EntityID : uint16_t
{
    PLAYER,
    BULLET,
    ROCK,
    HOUSE,
    STATIC_CAMERA, // In this example the camera is static and not attached to a entity
};

enum class MapID : uint8_t
{
    LEVEL_1
};

// The game class
struct Asteroids final : magique::Game
{
    Asteroids() : Game("Asteroids") {}
    void onStartup(magique::AssetLoader &al, magique::GameConfig &config) override;
    void onCloseEvent() override;

    void updateGame(entt::registry &registry) override;
    void drawWorld(Camera2D &camera) override;
    void drawGame(entt::registry &registry, Camera2D &camera) override;
    void drawUI() override;
};


struct PlayerScript final : magique::EntityScript
{
    void onKeyEvent(entt::registry &registry, entt::entity self) override;
};


struct BulletScript final : magique::EntityScript
{
    void onTick(entt::registry &registry, entt::entity self) override;
};

struct HouseScript final : magique::EntityScript
{
    void onDynamicCollision(entt::registry &registry, entt::entity self, entt::entity other) override;
};


#endif // ASTEROIDS_H