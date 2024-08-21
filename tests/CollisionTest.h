#ifndef COLLISIONTEST_H
#define COLLISIONTEST_H

#include <raylib/raylib.h>

#include <magique/core/Game.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/core/Core.h>
#include <magique/core/Draw.h>

using namespace magique;

enum EntityID : uint16_t
{
    PLAYER,
    OBJECT,
};

struct TestCompC final
{
    int counter = 0;
    bool isColliding = false;
};

struct PlayerScript final : EntityScript
{
    void onKeyEvent(entt::entity self) override
    {
        auto& pos = GetComponent<PositionC>(self);
        if (IsKeyDown(KEY_W))
            pos.y -= 2.5F;
        if (IsKeyDown(KEY_S))
            pos.y += 2.5F;
        if (IsKeyDown(KEY_A))
            pos.x -= 2.5F;
        if (IsKeyDown(KEY_D))
            pos.x += 2.5F;
    }

    void onDynamicCollision(entt::entity self, entt::entity other, const CollisionInfo& info) override
    {
        auto& pos = GetComponent<PositionC>(self);
        pos.x += info.normalVector.x * info.depth;
        pos.y += info.normalVector.y * info.depth;
    }
};

struct ObjectScript final : EntityScript // Moving platform
{
    void onTick(entt::entity self) override
    {
        auto& myComp = GetComponent<TestCompC>(self);
        myComp.isColliding = false;
        auto& test = GetComponent<TestCompC>(self);
        auto& pos = GetComponent<PositionC>(self);
        if (test.counter > 100)
        {
            pos.x++;
            if (test.counter >= 200)
            {
                test.counter = 0;
            }
        }
        else if (test.counter < 100)
        {
            pos.x--;
        }

        test.counter++;
    }

        void onDynamicCollision(entt::entity self, entt::entity other, const CollisionInfo&) override
        {
            auto& myComp = GetComponent<TestCompC>(self);
            myComp.isColliding = true;
        }
};

struct Test final : Game
{
    Test() : Game("magique - CollisionTest") {}
    void onStartup(AssetLoader& loader, GameConfig& config) override
    {
        SetShowHitboxes(true);
        const auto playerFunc = [](entt::entity e)
        {
            GiveActor(e);
            GiveScript(e);
            GiveCamera(e);
            GiveCollisionRect(e, 15, 25);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(PLAYER, playerFunc);
        const auto objFunc = [](entt::entity e)
        {
            GiveScript(e);
            const auto val = GetRandomValue(0,100);
            if (val < 25)
            {
                GiveCollisionRect(e, 25, 25);
            }
            else if (val < 50)
            {
                GiveCollisionTri(e, {-15, 15}, {15, 15});
            }
            else if (val < 75)
            {
                GiveCollisionCircle(e, 25);
            }
            else
            {
                GiveCollisionCapsule(e, 33, 15);
            }
            GetComponent<PositionC>(e).rotation = GetRandomValue(0, 360);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(OBJECT, objFunc);

        SetScript(PLAYER, new PlayerScript());
        SetScript(OBJECT, new ObjectScript());

        CreateEntity(PLAYER, 0, 0, MapID(0));
        for (int i = 0; i < 100; ++i)
        {
            CreateEntity(OBJECT, GetRandomValue(1,1000), GetRandomValue(1,1000), MapID(0));
        }
    }
    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D);
        DrawRectangle(250, 250, 50, 50, RED);
        for (const auto e : GetDrawEntities())
        {
            const auto& pos = GetComponent<const PositionC>(e);
            const auto& col = GetComponent<const CollisionC>(e);
            const auto& test = GetComponent<const TestCompC>(e);
            const auto color = test.isColliding ? PURPLE : BLUE;
            switch (col.shape)
            {
                [[likely]] case Shape::RECT: // Missing rotation anchor
                    DrawRectanglePro({pos.x, pos.y, col.p1, col.p2}, {0, 0}, pos.rotation, color);
                break;
            case Shape::CIRCLE:
                DrawCircleV({pos.x + col.p1 , pos.y + col.p1 }, col.p1, color);
                break;
            case Shape::CAPSULE:
                DrawCapsule2D(pos.x, pos.y, col.p1, col.p2, color);
                break;
            case Shape::TRIANGLE:
                DrawTriangleRot({pos.x, pos.y}, {pos.x + col.p1, pos.y + col.p2}, {pos.x + col.p3, pos.y + col.p4},
                                pos.rotation, col.anchorX, col.anchorY, color);
                break;
            }
        }
        DrawHashGridDebug();
        EndMode2D();
    }
    void updateGame(GameState gameState) override
    {
        for(const auto e : GetUpdateEntities())
        {
            auto& pos = GetComponent<PositionC>(e);
        }
    }
};

#endif //COLLISIONTEST_H