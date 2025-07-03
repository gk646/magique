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
    PLAYER,
    HUNTER,
};

constexpr float MOVE_SPEED = 2.5;
std::vector<Point> PATH{};
std::vector<Point> PATH2{};
ManualColliderGroup COLLIDERS;

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
    void onTick(entt::entity self, bool updated) override
    {
        auto& pos = GetComponent<PositionC>(self);
        const auto targetEntity = GetCameraEntity();
        auto& tarPos = GetComponent<PositionC>(targetEntity); // Player is the camera holder here

        auto mid = pos.getMiddle(GetComponent<CollisionC>(self));
        auto target = tarPos.getMiddle(GetComponent<CollisionC>(targetEntity));


        if (mid.euclidean(target) < 27)
            return;

        StartTimer(0);
        for (int i = 0; i < 100; ++i)
        {
            if (!FindPath(PATH, mid, target, pos.map, 1'000))
            {
                LOG_WARNING("Could not find path!");
                return;
            }
        }

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
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        // Define the objects and how they are created
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            GiveActor(e); // Don't forget to give your player the actor component!
            GiveCamera(e);
            GiveCollisionRect(e, 25, 25);
        };
        RegisterEntity(PLAYER, playerFunc);

        const auto objFunc = [](entt::entity e, EntityType type) { GiveCollisionRect(e, 25, 25); };
        RegisterEntity(HUNTER, objFunc);

        // Set the scripts
        SetEntityScript(PLAYER, new PlayerScript());
        SetEntityScript(HUNTER, new Hunter());

        // Create some objects - hunter is randomly positioned
        const MapID map = MapID::DEFAULT;
        CreateEntity(PLAYER, 50, 500, map);
        CreateEntity(HUNTER, 50, 50, map);


        float size = 48;
        // #######
        // #     #
        // ##### #
        // #     #
        // # #####
        // #     #
        // ##### #
        // #     #
        // # #####
        // Row 0: #######





        AddColliderGroup(MapID::DEFAULT, COLLIDERS);

        SetShowCompassOverlay(true);
        SetShowPathFindingOverlay(true);
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D);
        {
            for (const auto e : GetDrawEntities())
            {
                const auto& pos = GetComponent<const PositionC>(e);
                const auto& col = GetComponent<const CollisionC>(e);
                DrawRectangleRec({pos.x, pos.y, col.p1, col.p2}, BLUE);
            }

            const auto& pos = GetComponent<const PositionC>(GetCameraEntity());
            const auto& tarPos = GetComponent<const PositionC>(entt::entity(1)); // Unsafe

            // Solid objects
            for (const auto& col : COLLIDERS.getColliders())
            {
                DrawRectangleRec({col.x, col.y, col.p1, col.p2}, DARKGRAY);
            }

            DrawPath(PATH, BLUE);
            DrawPath(PATH2, GREEN);
        }
        EndMode2D();
    }
};

#endif //MAGIQUE_PATHFINDING_EXAMPLE_H