#ifndef COLLISION_BENCHMARK_H
#define COLLISION_BENCHMARK_H

#include <raylib/raylib.h>

#include <magique/magique.hpp>

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

enum class EntityType : uint16_t
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
        // info.penDepth = 0;
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
        EntityCreate(EntityType::OBJECT, Point::Random(0,4000), MapID(0));
    }
    EntityCreate(EntityType::PLAYER, {2500, 2500}, MapID(0));
    EngineSetBenchmarkTicks(300);
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
            EntityCreate(EntityType::OBJECT, {start + OBJECT_SIZE * j, OBJECT_SIZE * i}, MapID(0));
            total++;
        }
        currWidth += 2;
    }
    EntityCreate(EntityType::PLAYER, {0, -50}, MapID(0));
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
            ComponentGiveActor(e);

            ComponentGiveCamera(e);
            GiveCollisionRect(e, 15, 25);
            ComponentGive<TestCompC>(e);
        };
        EntityRegister(EntityType::PLAYER, playerFunc);
        const auto objFunc = [](entt::entity e, EntityType type)
        {
            const auto val = GetRandomValue(0, MAX_SHAPE);
            if (val < 25)
            {
                GiveCollisionRect(e, OBJECT_SIZE, OBJECT_SIZE);
            }
            else if (val < 50)
            {
                ComponentGiveCollision(e, {-15, 15}, {15, 15});
            }
            else
            {
                ComponentGiveCollision(e, 24);
            }

            ComponentGet<PositionC>(e).rotation = GetRandomValue(0, 5);
            ComponentGive<TestCompC>(e);
        };
        EntityRegister(EntityType::OBJECT, objFunc);
        ScriptingSetScript(EntityType::PLAYER, new PlayerScript());
        ScriptingSetScript(EntityType::OBJECT, new ObjectScript());

        benchmarkSetup();
        // pyramid();
    }

    void onDrawGame(GameState gameState, Camera2D& camera2D) override
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
                DrawRectanglePro({pos.pos.x, pos.pos.y, col.p1, col.p2}, {0, 0}, pos.rotation, color);
                break;
            case Shape::CIRCLE:
                DrawCircleV({pos.pos.x + col.p1, pos.pos.y + col.p1}, col.p1, color);
                break;
            case Shape::TRIANGLE:
                DrawTriangleRot({pos.pos.x, pos.pos.y}, {pos.pos.x + col.p1, pos.pos.y + col.p2},
                                {pos.pos.x + col.p3, pos.pos.y + col.p4}, pos.rotation, col.anchor, color);
                break;
            }
        }
        EndMode2D();
    }

    void onUpdateGame(GameState gameState) override
    {
        printf("Loaded Objects: %d\n", EngineGetUpdateEntities().size());
    }
};

#endif // COLLISION_BENCHMARK_H
