#define CATCH_CONFIG_MAIN

#include "raylib.h"


#include <magique/magique.hpp>

#include <entt/entity/registry.hpp>

#include <magique/assets/AssetPacker.h>

enum class EntityType : uint16_t
{
    PLAYER,
    ENEMY,
};


using namespace magique;

int main()
{
    Game myGame;

    return myGame.run();
}