#ifndef MAGEQUEST_H
#define MAGEQUEST_H

#include <entt/entity/registry.hpp>
#include <magique/core/Types.h>
#include <magique/ecs/Registry.h>
#include <magique/game/Game.h>


enum class EntityType : uint16_t
{
    PLAYER,
    ENEMY,
};

enum magique::MapID : uint8_t
{
    LEVEL_1,
};


using namespace magique;
class MageQuest final : public Game
{
    void onStartup(GameLoader& gl) override
    {
        gl.registerTask([](AssetContainer&) { printf("hey"); }, BACKGROUND_THREAD);

        gl.registerTask(
            [](AssetContainer&)
            {
                ecs::RegisterEntity(EntityType::PLAYER,
                                    [](entt::registry&, entt::entity e)
                                    {
                                        ecs::GiveActor(e);
                                        ecs::GiveCollision(e, AABB, 25, 50);
                                        ecs::GiveDebugVisuals(e);
                                        ecs::GiveDebugController(e);
                                    });
                ecs::RegisterEntity(EntityType::ENEMY,
                                    [](entt::registry&, entt::entity e)
                                    {
                                        ecs::GiveCollision(e, AABB, 25, 50);
                                        ecs::GiveDebugVisuals(e);
                                    });
            },
            MAIN_THREAD);
    }
    void drawGame(entt::registry& registry, Camera2D& camera) override
    {
        auto view = registry.view<PositionC, CollisionC, DebugVisualsC>();

        for (const auto e : view)
        {
            auto& pos = view.get<PositionC>(e);
            auto& col = view.get<CollisionC>(e);

            DrawRectangle(pos.x, pos.y, col.width, col.height, pos.type == EntityType::PLAYER ? BLUE : RED);
        }



    }


    void updateGame(entt::registry& registry) override
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
                ecs::CreateEntity(EntityType::PLAYER, 250, 250, LEVEL_1);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
                ecs::CreateEntity(EntityType::ENEMY, 450, 250, LEVEL_1);
        }

        auto view = registry.view<PositionC, DebugControllerC>();
        for (const auto e : view)
        {
            auto& pos = view.get<PositionC>(e);
            if (IsKeyDown(KEY_W))
                pos.y -= 5;
            if (IsKeyDown(KEY_S))
                pos.y += 5;
            if (IsKeyDown(KEY_A))
                pos.x -= 5;
            if (IsKeyDown(KEY_D))
                pos.x += 5;
        }
    }
};


#endif //MAGEQUEST_H