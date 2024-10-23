#ifndef MAGIQUE_PATHFINDING_EXAMPLE_H
#define MAGIQUE_PATHFINDING_EXAMPLE_H
#define MAGIQUE_EXAMPLE

#include <raylib/raylib.h>

#include <magique/core/Game.h>
#include <magique/core/Core.h>
#include <magique/core/Debug.h>
#include <magique/ecs/Scripting.h>
#include <magique/gamedev/PathFinding.h>

using namespace magique; // Is safe and recommended

enum class MapID : uint8_t
{
    DEFAULT,
};

enum EntityType : uint16_t
{
    PLAYER,
    HUNTER,
};

constexpr float MOVE_SPEED = 2.5F;

struct PlayerScript final : EntityScript
{
    void onKeyEvent(entt::entity self) override
    {
        auto& pos = GetComponent<PositionC>(self);
        if (IsKeyDown(KEY_W))
            pos.y -= MOVE_SPEED;
        if (IsKeyDown(KEY_S))
            pos.y += MOVE_SPEED;
        if (IsKeyDown(KEY_A))
            pos.x -= MOVE_SPEED;
        if (IsKeyDown(KEY_D))
            pos.x += MOVE_SPEED;
    }
};

struct Hunter final : EntityScript
{
    void onTick(entt::entity self) override
    {
        auto& pos = GetComponent<PositionC>(self);
        auto& tarPos = GetComponent<PositionC>(GetCameraEntity()); // Player is the camera holder here

        Point nextPoint = GetNextOnPath(pos.getPosition(), tarPos.getPosition(), pos.map);
        Point moveVec = GetDirectionVector(pos.getPosition(), nextPoint);
        pos.x += moveVec.x * MOVE_SPEED;
        pos.y += moveVec.y * MOVE_SPEED;
    }
};

struct Example final : Game
{
    Example() : Game("magique - PathFindingExample") {}

    void onStartup(AssetLoader& loader) override
    {
        SetTypePathSolid(HUNTER, true);
        const auto res = GetIsTypePathSolid(HUNTER);

        // Define the objects and how they are created
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            GiveActor(e); // Don't forget to give your player the actor component!
            GiveScript(e);
            GiveCamera(e);
            GiveCollisionRect(e, 25, 25);
        };
        RegisterEntity(PLAYER, playerFunc);

        const auto objFunc = [](entt::entity e, EntityType type)
        {
            GiveScript(e);
            GiveCollisionRect(e, 25, 25);
        };
        RegisterEntity(HUNTER, objFunc);

        // Set the scripts
        SetEntityScript(PLAYER, new PlayerScript());
        SetEntityScript(HUNTER, new Hunter());

        // Create some objects - hunter is randomly positioned
        const MapID map = MapID::DEFAULT;
        CreateEntity(PLAYER, 0, 0, map);
        CreateEntity(HUNTER, GetRandomValue(250, 1000), GetRandomValue(250, 1000), map);
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D);
        DrawRectangle(0, 0, 50, 50, RED); // Orientation
        for (const auto e : GetDrawEntities())
        {
            const auto& pos = GetComponent<const PositionC>(e);
            const auto& col = GetComponent<const CollisionC>(e);
            DrawRectangleRec({pos.x, pos.y, col.p1, col.p2}, BLUE);
        }
        EndMode2D();
        DrawPathFindingGrid(MapID::DEFAULT);
        Draw2DCompass(GREEN);
    }
};

#endif //MAGIQUE_PATHFINDING_EXAMPLE_H