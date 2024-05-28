#define CATCH_CONFIG_MAIN


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