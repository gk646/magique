#ifndef MAGIQUE_COLLISION_EXAMPLE_H
#define MAGIQUE_COLLISION_EXAMPLE_H

#include <magique/magique.hpp>

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
        auto& pos = ComponentGet<PositionC>(self); // Retrieve the position component - implicit for each entity
        if (IsKeyDown(KEY_W))
            pos.pos.y -= speed;
        if (IsKeyDown(KEY_S))
            pos.pos.y += speed;
        if (IsKeyDown(KEY_A))
            pos.pos.x -= speed;
        if (IsKeyDown(KEY_D))
            pos.pos.x += speed;
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
        auto& myComp = ComponentGet<TestCompC>(self);
        myComp.isColliding = false; // Reset collision flag
        auto& pos = ComponentGet<PositionC>(self);

        if (DISABLE_MOVEMENT)
            return;
        if (myComp.counter > 100)
        {
            pos.pos.x++;
            if (myComp.counter >= 200)
            {
                myComp.counter = 0;
            }
        }
        else if (myComp.counter < 100)
        {
            pos.pos.x--;
        }

        myComp.counter++;
    }

    // Called if this any entity of this type collides with any other entity
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override
    {
        ComponentGet<TestCompC>(self).isColliding = true; // Set the collide flag to color the entity
       AccumulateCollision(info);                        // Accumulates the collision info to resolve the collision
    }
};

struct Example final : Game
{
    Example() : Game("magique - CollisionTest") {}

    void onStartup(AssetLoader& loader) override
    {
        EngineSetState({});               // Set empty gamestate - needs to be set in a real game
        SetShowHitboxes(true);          // Show all collision hitboxes
        SetShowEntityGridOverlay(true); // Shows the size and entity count of each cell in the entity collision grid
        // Create the player
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            ComponentGiveActor(e);
            ComponentGiveCamera(e);
            GiveCollisionRect(e, 25, 25);
            ComponentGive<TestCompC>(e);
            auto& pos = ComponentGet<PositionC>(e);
            pos.rotation = 10;
        };
        EntityRegister(Player, playerFunc);

        // Create moving objects
        const auto objFunc = [](entt::entity e, EntityType type)
        {
            const int shapeNum = GetRandomValue(0, 100);
            if (shapeNum < 25)
            {
                GiveCollisionRect(e, 25, 25);
            }
            else if (shapeNum < 50)
            {
                ComponentGiveCollision(e, {-33, 33}, {33, 33});
            }
            else if (shapeNum <= 100)
            {
                ComponentGiveCollision(e, 25);
            }
            ComponentGet<PositionC>(e).rotation = GetRandomValue(0, 360);
            ComponentGive<TestCompC>(e);
        };
        EntityRegister(OBJECT, objFunc);

        ScriptingSetScript(Player, new PlayerScript());
        ScriptingSetScript(OBJECT, new ObjectScript());

        auto player = EntityCreate(Player, {0, 0}, MapID(0));
        for (int i = 0; i < 25; ++i)
        {
            EntityCreate(OBJECT, Point::Random(-500, 500), MapID(0));
        }

    }

    void onDrawGame(GameState /**/, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D); // Start drawing in respect to the camera

        DrawRectangle(250, 250, 250, 75, RED);
        auto& font = EngineGetFont();
        DrawTextEx(font, "This is stationary object\n for reference", {250, 250}, 17, 1, WHITE);
        DrawTextEx(font, "Shape and Rotation of all objects is random", {0, -25}, 17, 1, BLACK);
        DrawTextEx(font, "Press SPACE to toggle moving obstacles", {0, -50}, 17, 1, BLACK);

        for (const auto e : GetDrawEntities())
        {
            const auto& pos = ComponentGet<const PositionC>(e);
            const auto& col = ComponentGet<const CollisionC>(e);
            const auto& test = ComponentGet<const TestCompC>(e);
            const auto color = test.isColliding ? PURPLE : BLUE;
            switch (col.shape)
            {
            [[likely]] case Shape::RECT: // Missing rotation anchor
                {
                    auto origin = Point{col.p1, col.p2} / 2;
                    auto [x, y] = pos.pos + origin;
                    DrawRectanglePro({x, y, col.p1, col.p2}, origin, pos.rotation, color);
                }
                break;
            case Shape::CIRCLE:
                DrawCircleV({pos.pos.x + col.p1, pos.pos.y + col.p1}, col.p1, color);
                break;
            case Shape::TRIANGLE:
                DrawTriangleRot({pos.pos.x, pos.pos.y}, {pos.pos.x + col.p1, pos.pos.y + col.p2},
                                {pos.pos.x + col.p3, pos.pos.y + col.p4}, pos.rotation, col.anchorX, col.anchorY, color);
                break;
            }
        }

        EndMode2D(); // End drawing with the camera
    }

    void onUpdateGame(GameState gameState) override
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            DISABLE_MOVEMENT = !DISABLE_MOVEMENT;
        }
    }
};


#endif // MAGIQUE_COLLISION_EXAMPLE_H
