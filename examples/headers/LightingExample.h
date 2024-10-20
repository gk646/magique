#ifndef MAGIQUE_LIGHTING_EXAMPLE
#define MAGIQUE_LIGHTING_EXAMPLE
#define MAGIQUE_EXAMPLE

#include <raylib/raylib.h>

#include <magique/core/Game.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/core/Core.h>
#include <magique/core/Debug.h>
#include <magique/core/Draw.h>

using namespace magique;

enum EntityType : uint16_t
{
    PLAYER,
    OBJECT,
};

struct MoveCompC final
{
    int counter = 0;
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
};

struct ObjectScript final : EntityScript // Moving platform
{
    void onTick(entt::entity self) override
    {
        auto& test = GetComponent<MoveCompC>(self);
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
};

struct Example final : Game
{
    Example() : Game("magique - LightingExample") {}

    void onStartup(AssetLoader& loader) override
    {
        SetLightingMode(LightingMode::STATIC_SHADOWS); // Sets the lighting mode
        SetShowHitboxes(true);

        // Define the objects and how they are created
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            GiveActor(e);
            GiveScript(e);
            GiveCamera(e);
            GiveEmitter(e, RED, 3250, POINT_LIGHT_SOFT);
            GiveCollisionRect(e, 25, 25);
            GiveComponent<MoveCompC>(e);
        };
        RegisterEntity(PLAYER, playerFunc);

        const auto objFunc = [](entt::entity e, EntityType type)
        {
            GiveScript(e);
            GiveCollisionRect(e, 25, 25);
            GiveOccluder(e, 25, 25);
            GiveComponent<MoveCompC>(e);
        };
        RegisterEntity(OBJECT, objFunc);

        // Set the scripts
        SetEntityScript(PLAYER, new PlayerScript());
        SetEntityScript(OBJECT, new ObjectScript());

        // Create some objects
        CreateEntity(PLAYER, 0, 0, MapID(0));
        for (int i = 0; i < 25; ++i)
        {
            CreateEntity(OBJECT, GetRandomValue(1, 1000), GetRandomValue(1, 1000), MapID(0));
        }
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D);

        DrawRectangle(0, 0, 75, 75, RED); // Non-moving reference rectangles
        for (const auto e : GetDrawEntities())
        {
            const auto& pos = GetComponent<const PositionC>(e);
            const auto& col = GetComponent<const CollisionC>(e);
            DrawRectanglePro({pos.x, pos.y, col.p1, col.p2}, {0, 0}, pos.rotation, BLUE);
        }
        EndMode2D();
    }
};

#endif //MAGIQUE_LIGHTING_EXAMPLE