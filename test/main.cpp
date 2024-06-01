#define CATCH_CONFIG_MAIN

#include "MageQuest.h"


#include <magique/magique.hpp>
#include <magique/core/Core.h>
#include <entt/entity/registry.hpp>
#include <magique/assets/AssetPacker.h>
#include <magique/util/Logging.h>

enum class EntityType : uint16_t
{
    PLAYER,
    ENEMY,
};


using namespace magique;

int main()
{

    MageQuest myGame;
    magique::util::SetLogLevel(magique::util::LEVEL_INFO);
    ecs::RegisterEntity(EntityType::PLAYER,
                        [](entt::registry& reg, const entt::entity e)
                        {
                            ecs::MakeActor(e);
                            ecs::MakeCollision(e, AABB);
                        });

    const auto e = ecs::CreateEntity(EntityType::PLAYER);

    ecs::DestroyEntity(e);

    return myGame.run();
}