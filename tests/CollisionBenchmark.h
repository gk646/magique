#ifndef COLLISION_BENCHMARK_H
#define COLLISION_BENCHMARK_H

#include <raylib/raylib.h>

#include <magique/core/Game.h>
#include <magique/util/Logging.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/core/Core.h>
#include <magique/core/Debug.h>
#include <magique/core/Draw.h>

//-----------------------------------------------
// Collision Benchmark
//-----------------------------------------------
// .....................................................................
// With 4 threads and 50k dynamic entities
// Just some anecdotal average tick times when running for 5s (300 ticks)
// Time: 18.9ms | Start
// Time: 17.5ms | Added early return bounding-box check to CircleToQuadrilateral()
// Time: 17.1ms | Added early return bounding-box check to CircleToCapsule()
// Time: 16.1ms | Used view.get<> instead of registry.get<>
// Time: 14.3ms | Added early return bounding-box check to SAT()
// Time: 9.3 ms | Using a single hashgrid with 50 width
// Time: 9.1 ms | Fixing circle calculations / removing unneeded division (top left + radius not radius/2 lol)
// Time: 7.6 ms | Changed access pattern to hashgrid from queries to direct cell by cell iteration -> more cache local
// Time: 6.9 ms | Changed ratio of work for main thread (get more) 39.5% main vs 60.5 distributed among workers (with 4)
// Time: 6.7 ms | Optimized ECS access further using groups
// Fixed update area being wrong -> more objects are loaded now
// Time: 9.1 ms | New Baseline
// Time: 8.9 ms | Removed EnTT asserts in release builds (Edit: i just didnt define NDEBUG ...)
// Time: 8.5 ms | Removed immediate collision resolving - its accumulated per user call
// Time: 9.3 ms | Added static collisions
// Time: 8.6 ms | Added power of two optimization
// Time: 8.9 ms | Fixed Bounding box calculations for non rotated triangles -> more shapes -> bit slower
// Time: 9.8 ms | Added accumulation of collision info using a hashmap
// Time: 9.2 ms | Removed hashmap in favor of caching data and saving it inside the collision component
// Time: 7.4 ms | Switched to Linux + GCC 14.2 + all optimizations
// Time: 7.1 ms | Upgrade to entt 3.14
// Some months later
// Time: 4.7 ms | Fedora 42 + GCC 15.1 + performance mode + ?
// Time: 4.9 ms | Fixing the pointer invalidation - need to refetch the components
// Time: 4.5 ms | Re-enabled SIMD - accidentally had wrong define
// Changed default hash size to be smaller & more max entities per cell -> actually all object are checked now
// Time: 6.0 ms | Added sticky edge avoidance
// Changed to only 2 extra threads
// Time: 7.68
// Time: 7.54   | optimized iteration and removed a branch
// Time: 5.69ms | New compiler version? some minor branching optimizations
// .....................................................................

using namespace magique;

enum EntityType : uint16_t
{
    PLAYER,
    OBJECT,
};

struct TestCompC final
{
    bool isColliding = false;
};

struct PlayerScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override
    {
        auto& myComp = ComponentGet<TestCompC>(self);
        myComp.isColliding = false;
    }
    void onKeyEvent(entt::entity self) override
    {
        auto& pos = ComponentGet<PositionC>(self);
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
        auto& myComp = ComponentGet<TestCompC>(self);
        myComp.isColliding = true;
        //  AccumulateCollision(info);
    }
};

struct ObjectScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override
    {
        auto& myComp = ComponentGet<TestCompC>(self);
        myComp.isColliding = false;
    }
    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override
    {
        auto& myComp = ComponentGet<TestCompC>(self);
        myComp.isColliding = true;
        //info.penDepth = 0;
        // info.normalVector = {0, 0};
        // AccumulateCollision(info);
    }
};

int MAX_SHAPE = 100;
float OBJECT_SIZE = 25;

void benchmarkSetup()
{
    for (int i = 0; i < 50'000; ++i)
    {
        CreateEntity(OBJECT, GetRandomValue(0, 4000), GetRandomValue(0, 4000), MapID(0));
    }
    CreateEntity(PLAYER, 2500, 2500, MapID(0));
    SetBenchmarkTicks(300);
}

void pyramid()
{
    MAX_SHAPE = 0;
    OBJECT_SIZE = 10.0F;
    int currWidth = 1;
    int total = 0;
    for (int i = 0; i < 150; ++i)
    {
        float start = 0 - int(currWidth / 2) * OBJECT_SIZE;
        for (int j = 0; j < currWidth; ++j)
        {
            CreateEntity(OBJECT, start + OBJECT_SIZE * j, OBJECT_SIZE * i, MapID(0));
            total++;
        }
        currWidth += 2;
    }
    CreateEntity(PLAYER, 0, -50, MapID(0));
    LOG_INFO("Spawned %d boxes", total);
}

struct Example final : Game
{
    Example() : Game("magique - CollisionBenchmark") {}

    void onStartup(AssetLoader& loader) override
    {
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        SetRandomSeed(100);
        // SetShowHitboxes(true);
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            GiveActor(e);

            GiveCamera(e);
            GiveCollisionRect(e, 15, 25);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(PLAYER, playerFunc);
        const auto objFunc = [](entt::entity e, EntityType type)
        {
            const auto val = GetRandomValue(0, MAX_SHAPE);
            if (val < 25)
            {
                GiveCollisionRect(e, OBJECT_SIZE, OBJECT_SIZE);
            }
            else if (val < 50)
            {
                GiveCollisionTri(e, {-15, 15}, {15, 15});
            }
            else if (val < 75)
            {
                GiveCollisionCircle(e, 24);
            }
            else
            {
                GiveCollisionCapsule(e, 33, 15);
            }
            ComponentGet<PositionC>(e).rotation = GetRandomValue(0, 5);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(OBJECT, objFunc);
        SetEntityScript(PLAYER, new PlayerScript());
        SetEntityScript(OBJECT, new ObjectScript());

        benchmarkSetup();
        //pyramid();
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D);
        for (const auto e : EngineGetDrawEntities())
        {
            const auto& pos = ComponentGet<const PositionC>(e);
            const auto& col = ComponentGet<const CollisionC>(e);
            const auto& test = ComponentGet<const TestCompC>(e);
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
        EndMode2D();
    }

    void updateGame(GameState gameState) override
    {
        //printf("Loaded Objects: %d\n", GetUpdateEntities().size());
    }
};

#endif //COLLISION_BENCHMARK_H
