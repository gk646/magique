#ifndef MAGEQUEST_H
#define MAGEQUEST_H

#include "../../cxstructs/src/cxconfig.h"
#include "../../cxstructs/src/cxutil/cxtime.h"


#include <magique/assets/AssetManager.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/ecs/Registry.h>
#include <magique/core/Game.h>
#include <magique/core/Draw.h>


enum class EntityType : uint16_t
{
    PLAYER,
    ENEMY,
};

enum magique::MapID : uint8_t
{
    LEVEL_1,
};


enum HandleType
{
    GENZ,
};

using namespace magique;
struct MageQuest final : Game
{
    void onStartup(GameLoader& gl) override
    {

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
            BACKGROUND_THREAD);

        gl.registerTask([](AssetContainer& assets)
                        { RegisterHandle(RegisterTexture(assets.GetAsset("genz_old.png")), GENZ); }, MAIN_THREAD);
    }

    void drawGame(entt::registry& registry, Camera2D& camera) override
    {
        const auto view = registry.view<PositionC, CollisionC, DebugVisualsC>();
        for (const auto e : view)
        {
            auto& pos = view.get<PositionC>(e);
            auto& col = view.get<CollisionC>(e);

            DrawRectangle(pos.x, pos.y, col.width, col.height, pos.type == EntityType::PLAYER ? BLUE : RED);
        }

        DrawRegion(GetTextureRegion(GetHandle(GENZ)), 50, 50, WHITE);
    }

    void updateGame(entt::registry& registry) override
    {
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

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            srand(150);
            for (int i = 0; i < 1'000; ++i)
            {
                ecs::CreateEntity(EntityType::PLAYER, rand() % 1111, rand() % 1111, LEVEL_1);
            }
            return;
            for (int i = 0; i < 50; ++i)
            {

                ecs::CreateEntity(EntityType::PLAYER, 250, 250, LEVEL_1);
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            for (int i = 0; i < 100; ++i)
            {
                ecs::CreateEntity(EntityType::ENEMY, rand() % 100000, rand() % 100000, LEVEL_1);
            }
        }
    }
};


#endif //MAGEQUEST_H