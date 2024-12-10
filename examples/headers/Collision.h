#ifndef MAGIQUE_COLLISION_EXAMPLE_H
#define MAGIQUE_COLLISION_EXAMPLE_H

#include <magique/core/Game.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/core/Core.h>
#include <magique/core/Debug.h>
#include <magique/core/Draw.h>

// Recommended
using namespace magique;

// Define the types of entities used in this game
enum EntityType : uint16_t
{
    PLAYER,
    OBJECT,
};

// Helper entity component to save some data
struct TestCompC final
{
    int counter = 0;
    bool isColliding = false;
};

// Custom Script to enable WASD movement
struct PlayerScript final : EntityScript
{
    // Called if any keystate changed
    void onKeyEvent(entt::entity self) override
    {
        auto& pos = GetComponent<PositionC>(self); // Retrieve the position component - implicit for each entity
        if (IsKeyDown(KEY_W))
            pos.y -= 2.5F;
        if (IsKeyDown(KEY_S))
            pos.y += 2.5F;
        if (IsKeyDown(KEY_A))
            pos.x -= 2.5F;
        if (IsKeyDown(KEY_D))
            pos.x += 2.5F;
    }

    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override
    {
        info.penDepth *= 0.5F;
        AccumulateCollision(info);
    }
};

struct ObjectScript final : EntityScript // Moving platform
{
    // Called at the beginning of each tick
    void onTick(entt::entity self, bool updated) override
    {
        auto& myComp = GetComponent<TestCompC>(self);
        myComp.isColliding = false; // Reset collision flag
        auto& pos = GetComponent<PositionC>(self);
        if (myComp.counter > 100)
        {
            pos.x++;
            if (myComp.counter >= 200)
            {
                myComp.counter = 0;
            }
        }
        else if (myComp.counter < 100)
        {
            pos.x--;
        }

        myComp.counter++;
    }

    // Called if this any entity of this type collides with any other entity
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override
    {
        GetComponent<TestCompC>(self).isColliding = true; // Set the collide flag to color the entity
        info.penDepth *= 0.5F;
        AccumulateCollision(info); // Accumulates the collision info to resolve the collision
    }
};

struct Example final : Game
{
    Example() : Game("magique - CollisionTest") {}

    void onStartup(AssetLoader& loader) override
    {
        SetShowHitboxes(true); // Show all collision hitboxes
        // Create the player
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            GiveActor(e);
            GiveCamera(e);
            GiveCollisionRect(e, 20, 20);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(PLAYER, playerFunc);

        // Create moving objects
        const auto objFunc = [](entt::entity e, EntityType type)
        {
            const auto val = GetRandomValue(0, 100);
            if (val < 25)
            {
                GiveCollisionRect(e, 25, 25);
            }
            else if (val < 50)
            {
                GiveCollisionTri(e, {-33, 33}, {33, 33});
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

        SetEntityScript(PLAYER, new PlayerScript());
        SetEntityScript(OBJECT, new ObjectScript());

        CreateEntity(PLAYER, 0, 0, MapID(0));
        for (int i = 0; i < 25; ++i)
        {
            CreateEntity(OBJECT, GetRandomValue(-500, 500), GetRandomValue(-500, 500), MapID(0));
        }
    }

    void drawGame(GameState /**/, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D); // Start drawing in respect to the camera

        DrawRectangle(250, 250, 250, 75, RED);
        DrawTextEx(GetFont(), "This is stationary object\n for reference", {250, 250}, 17, 1, WHITE);
        DrawTextEx(GetFont(), "Shape and Rotation of all objects is random", {0, -25}, 17, 1, BLACK);

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
                DrawCircleV({pos.x + col.p1, pos.y + col.p1}, col.p1, color);
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

        DrawHashGridDebug(GetCameraMap()); // Draws the debug grid and entity count for the internal hashgrid

        EndMode2D(); // End drawing with the camera
    }

    void updateGame(GameState gameState) override
    {
        for (const auto e : GetUpdateEntities())
        {
            auto& pos = GetComponent<PositionC>(e);
        }
    }
};


#endif // MAGIQUE_COLLISION_EXAMPLE_H