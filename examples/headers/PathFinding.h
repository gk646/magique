#ifndef MAGIQUE_PATHFINDING_EXAMPLE_H
#define MAGIQUE_PATHFINDING_EXAMPLE_H

#include "magique/core/Camera.h"
#include "magique/util/Math.h"

#include <raylib/raylib.h>

#include <magique/core/Game.h>
#include <magique/util/Logging.h>
#include <magique/core/Core.h>
#include <magique/core/Debug.h>
#include <magique/core/StaticCollision.h>
#include <magique/ecs/Scripting.h>
#include <magique/gamedev/PathFinding.h>

using namespace magique; // Is safe and recommended

enum class MapID : uint8_t
{
    DEFAULT,
};

enum EntityType : uint16_t
{
    Player,
    OBSTACLE,
    HUNTER,
};

constexpr float MOVE_SPEED = 2.5;
std::vector<Point> PATH{};
bool STOP_FLAG = false;

struct PlayerScript final : EntityScript
{
    void onKeyEvent(entt::entity self) override
    {
        auto& pos = ComponentGet<PositionC>(self);
        if (IsKeyDown(KEY_W))
            pos.y -= MOVE_SPEED;
        if (IsKeyDown(KEY_S))
            pos.y += MOVE_SPEED;
        if (IsKeyDown(KEY_A))
            pos.x -= MOVE_SPEED;
        if (IsKeyDown(KEY_D))
            pos.x += MOVE_SPEED;

        if (IsKeyPressed(KEY_SPACE))
            STOP_FLAG = !STOP_FLAG;
    }

    void onMouseEvent(entt::entity self) override
    {
        auto worldMouse = GetScreenToWorld2D(GetMousePosition(), GetCamera());
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (GetNearbyEntities({}, {worldMouse.x, worldMouse.y}, 1).empty())
                EntityCreate(OBSTACLE, worldMouse.x, worldMouse.y, {});
        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            auto& vec = GetNearbyEntities({}, {worldMouse.x, worldMouse.y}, 1);
            for (const auto e : vec)
            {
                EntityDestroy(e);
            }
        }
    }
};

struct ObstacleScript final : EntityScript
{
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override {}
};

struct HunterScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override
    {

        auto& pos = ComponentGet<PositionC>(self);
        const auto targetEntity = GetCameraEntity();
        auto& tarPos = ComponentGet<PositionC>(targetEntity); // Player is the camera holder here

        auto mid = pos.getMiddle(ComponentGet<CollisionC>(self));
        auto target = tarPos.getMiddle(ComponentGet<CollisionC>(targetEntity));

        for (int i = 0; i < 200; ++i)
        {
            if (GetEngineTick() % 10 != 0)
                continue;
            if (!FindPath(PATH, mid, target, pos.map, 150, GridMode::STAR))
            {
                LOG_WARNING("Could not find path!");
            }
        }

        if (STOP_FLAG || PATH.empty())
            return;

        Point nextPoint = PATH[PATH.size() - 1];
        Point moveVec = GetDirectionVector(mid, nextPoint);

        pos.x += moveVec.x * MOVE_SPEED;
        pos.y += moveVec.y * MOVE_SPEED;
    }
};

struct Example final : Game
{
    Example() : Game("magique - PathFindingExample") {}

    void onStartup(AssetLoader& loader) override
    {
        SetTargetFPS(100);
        //SetBenchmarkTicks(300);
        // Define the objects and how they are created
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            ComponentGiveActor(e); // Don't forget to give your player the actor component!
            ComponentGiveCamera(e);
            GiveCollisionRect(e, 25, 25);
        };
        EntityRegister(Player, playerFunc);

        const auto hunterFunc = [](entt::entity e, EntityType type) { GiveCollisionRect(e, 25, 25); };
        EntityRegister(HUNTER, hunterFunc);

        const auto objFunc = [](entt::entity e, EntityType type) { GiveCollisionRect(e, 50, 50); };
        EntityRegister(OBSTACLE, objFunc);

        // Set the scripts
        ScriptingSetScript(Player, new PlayerScript());
        ScriptingSetScript(HUNTER, new HunterScript());
        ScriptingSetScript(OBSTACLE, new ObstacleScript());

        // Create some objects
        const MapID map = MapID::DEFAULT;
        EntityCreate(Player, 50, 300, map);
        EntityCreate(HUNTER, 50, 50, map);

        SetShowCompassOverlay(true);
        SetShowPathFindingOverlay(true);
        SetTypePathSolid(OBSTACLE, true);
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D);
        {
            for (const auto e : GetDrawEntities())
            {
                const auto& pos = ComponentGet<const PositionC>(e);
                const auto& col = ComponentGet<const CollisionC>(e);
                auto color = pos.type == OBSTACLE ? BLACK : pos.type == HUNTER ? RED : GREEN;
                DrawRectangleRec({pos.x, pos.y, col.p1, col.p2}, color);
            }
        }
        DrawPath(PATH);
        EndMode2D();
    }
};

#endif //MAGIQUE_PATHFINDING_EXAMPLE_H
