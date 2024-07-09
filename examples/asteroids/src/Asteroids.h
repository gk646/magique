#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <magique/core/Game.h>

enum EntityID : uint16_t
{
    PLAYER,
    ROCK_SMALL,
    ROCK_MEDIUM,
    ROCK_LARGE,
    ROCK_EXPLODE,
    ALIEN_NORMAL,
    ALIEN_MEDIUM,
    ALIEN_BOSS,
};

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

#endif // ASTEROIDS_H