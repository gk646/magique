#ifndef MAGIQUE_COLLISION_EXAMPLE_H
#define MAGIQUE_COLLISION_EXAMPLE_H

#include <magique/core/Camera.h>
#include <magique/core/Game.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/core/Core.h>
#include <magique/core/Debug.h>
#include <magique/core/Draw.h>
#include <magique/util/Logging.h>

// Recommended
using namespace magique;

// Define the types of entities used in this game
enum EntityType : uint16_t
{
    Player,
    OBJECT,
};

// Helper entity component to save some data
struct TestCompC final
{
    int counter = 0;
    bool isColliding = false;
};

bool DISABLE_MOVEMENT = false;

// Custom Script to enable WASD movement
struct PlayerScript final : EntityScript
{
    // Called if any keystate changed
    void onKeyEvent(entt::entity self) override
    {
        float speed = 2.0F;
        auto& pos = GetComponent<PositionC>(self); // Retrieve the position component - implicit for each entity
        if (IsKeyDown(KEY_W))
            pos.y -= speed;
        if (IsKeyDown(KEY_S))
            pos.y += speed;
        if (IsKeyDown(KEY_A))
            pos.x -= speed;
        if (IsKeyDown(KEY_D))
            pos.x += speed;
    }

    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override
    {
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

        if (DISABLE_MOVEMENT)
            return;
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
        // AccumulateCollision(info);                        // Accumulates the collision info to resolve the collision
    }
};

struct Example final : Game
{
    Example() : Game("magique - CollisionTest") {}

    void onStartup(AssetLoader& loader) override
    {
        SetGameState({});               // Set empty gamestate - needs to be set in a real game
        SetShowHitboxes(true);          // Show all collision hitboxes
        SetShowEntityGridOverlay(true); // Shows the size and entity count of each cell in the entity collision grid
        // Create the player
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            GiveActor(e);
            GiveCamera(e);
            GiveCollisionRect(e,25, 25);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(Player, playerFunc);

        // Create moving objects
        const auto objFunc = [](entt::entity e, EntityType type)
        {
            const int shapeNum = GetRandomValue(0, 11);
            if (shapeNum < 25)
            {
                GiveCollisionRect(e, 25, 25);
            }
            else if (shapeNum < 50)
            {
                GiveCollisionTri(e, {-33, 33}, {33, 33});
            }
            else if (shapeNum < 75)
            {
                GiveCollisionCircle(e, 25);
            }
            else
            {
                GiveCollisionCapsule(e, 33, 15);
            }
            GetComponent<PositionC>(e).rotation = GetRandomValue(0, 0);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(OBJECT, objFunc);

        SetEntityScript(Player, new PlayerScript());
        SetEntityScript(OBJECT, new ObjectScript());

        auto player = CreateEntity(Player, 0, 0, MapID(0));
        for (int i = 0; i < 25; ++i)
        {
            // CreateEntity(OBJECT, GetRandomFloat(-500, 500), GetRandomFloat(-500, 500), MapID(0));
        }

        CreateEntity(OBJECT, 50, 0, MapID(0));
        CreateEntity(OBJECT, 50, 25, MapID(0));
        CreateEntity(OBJECT, 50, 50, MapID(0));
        CreateEntity(OBJECT, 50, 75, MapID(0));
        CreateEntity(OBJECT, 50, 100, MapID(0));

        CreateEntity(OBJECT, 25, -55, MapID(0));
        CreateEntity(OBJECT, 0, -100, MapID(0));
        GetComponent<CollisionC>(player).weight = 100;
    }

    void drawGame(GameState /**/, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D); // Start drawing in respect to the camera

        DrawRectangle(250, 250, 250, 75, RED);
        auto& font = GetEngineFont();
        DrawTextEx(font, "This is stationary object\n for reference", {250, 250}, 17, 1, WHITE);
        DrawTextEx(font, "Shape and Rotation of all objects is random", {0, -25}, 17, 1, BLACK);
        DrawTextEx(font, "Press SPACE to toggle moving obstacles", {0, -50}, 17, 1, BLACK);

        for (const auto e : GetDrawEntities())
        {
            continue;
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

        EndMode2D(); // End drawing with the camera
    }

    void updateGame(GameState gameState) override
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            DISABLE_MOVEMENT = !DISABLE_MOVEMENT;
        }
    }
};


#endif // MAGIQUE_COLLISION_EXAMPLE_H