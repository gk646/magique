#include "Asteroids.h"

#include <raylib/raylib.h>

#include <magique/assets/AssetLoader.h>
#include <magique/ecs/ECS.h>

void Asteroids::onStartup(magique::AssetLoader &al, magique::GameConfig &config)
{
    // Set FPS to 120
    SetTargetFPS(120);

    // Load the textures
    al.registerTask([](magique::AssetContainer &assets) {}, magique::MAIN_THREAD, magique::MEDIUM, 5);

    // Register the player entity
    magique::RegisterEntity(EntityID::PLAYER, [](entt::registry& registry, entt::entity entity) {});
}

void Asteroids::onCloseEvent() { shutDown(); }

void Asteroids::updateGame(entt::registry &registry) {}

void Asteroids::drawWorld(Camera2D &camera) {}

void Asteroids::drawGame(entt::registry &registry, Camera2D &camera) {}

void Asteroids::drawUI() {}