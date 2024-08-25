#ifndef RENDERUTIL_H
#define RENDERUTIL_H

void SetTargetFPS(const int fps) // raylib function implemented here
{
    auto& config = magique::global::ENGINE_CONFIG.timing;
    if (fps < 1)
        config.frameTarget = 0;
    else
    {
        config.frameTarget = 1.0 / static_cast<double>(fps);
        // Round down cause minimal accuracy is only 1ms - so wait 1 ms less to be accurate
        config.sleepTime = std::floor(config.frameTarget * 1000) / 1000;
        config.workPerTick = MAGIQUE_LOGIC_TICKS / static_cast<double>(fps);
    }
}

int GetFPS()
{
    auto& config = magique::global::ENGINE_CONFIG.timing;
#define FPS_BUFF_SIZE 15
    static double lastTime = 0;
    static int fpsBuffer[FPS_BUFF_SIZE] = {0};
    static int index = 0;
    static int sumFPS = 0;
    static float count = 0;

    const double currentTime = GetTime();

    const int currFPS = static_cast<int>(std::ceil(static_cast<double>(config.frameCounter) / (currentTime - lastTime)));

    sumFPS -= fpsBuffer[index];
    fpsBuffer[index] = currFPS;
    sumFPS += currFPS;
    index = (index + 1) % FPS_BUFF_SIZE;

    if (count < FPS_BUFF_SIZE)
    {
        count++;
    }
    const int ret = static_cast<int>(std::ceil(static_cast<float>(sumFPS) / count));

    lastTime = currentTime;
    config.frameCounter = 0;

    return ret;
}

// Get time in seconds for last frame drawn (delta time)
float GetFrameTime()
{
    const auto& perf = magique::global::PERF_DATA;
    return static_cast<float>(perf.drawTickTime + perf.logicTickTime) / 1'000'000'000.0F;
}

namespace magique
{
    inline void HandleLoadingScreen(Game& game)
    {
        auto& loader = global::LOADER;
        if (loader != nullptr) [[likely]]
        {
            auto& lScreen = *global::ENGINE_CONFIG.loadingScreen;
            const bool isStartup = loader->isStartup();
            const auto drawRes = lScreen.draw(isStartup, loader->getProgressPercent());
            const auto res = loader->step();
            if (res && drawRes)
            {
                delete loader;
                loader = nullptr;
                game.isLoading = false;
                if (isStartup)
                {
                    game.onLoadingFinished();
                    ResetBenchmarkTimes();
                }
            }
        }
        else
        {
            game.isLoading = false;
        }
    }

    inline void RenderHitboxes()
    {
        BeginMode2D(GetCamera());
        auto& group = internal::POSITION_GROUP;
        const auto& config = global::ENGINE_CONFIG;
        // Dynamic entities
        for (const auto e : GetDrawEntities())
        {
            if (!group.contains(e))
                continue;
            const auto& pos = group.get<const PositionC>(e);
            const auto& col = group.get<const CollisionC>(e);
            switch (col.shape)
            {
            [[likely]] case Shape::RECT:
                DrawRectangleLinesRot({pos.x, pos.y, col.p1, col.p2}, pos.rotation, col.anchorX, col.anchorY, RED);
                break;
            case Shape::CIRCLE:
                DrawCircleLinesV({pos.x + col.p1, pos.y + col.p1}, col.p1, RED);
                break;
            case Shape::CAPSULE:
                DrawCapsule2DLines(pos.x, pos.y, col.p1, col.p2, RED);
                break;
            case Shape::TRIANGLE:
                DrawTriangleLinesRot({pos.x, pos.y}, {pos.x + col.p1, pos.y + col.p2}, {pos.x + col.p3, pos.y + col.p4},
                                     pos.rotation, col.anchorX, col.anchorY, RED);
                break;
            }
        }
        // Static tile map objects
        for (const auto& [x, y, p1, p2] : global::STATIC_COLL_DATA.objectHolder.colliders)
        {
            if (p1 == 0)
                continue;
            if (p2 != 0)
            {
                DrawRectangleLinesEx({x, y, p1, p2}, 2, RED);
            }
            else
            {
                DrawCircleLinesV({x + p1, y + p1}, p1, RED);
            }
        }

        if (config.getIsWorldBoundSet()) // enabled
        {
            constexpr float depth = 250.0F;
            const auto wBounds = config.worldBounds;
            const Rectangle r1 = {wBounds.x - depth, wBounds.y - depth, depth, wBounds.height + depth};
            const Rectangle r2 = {wBounds.x, wBounds.y - depth, wBounds.width, depth};
            const Rectangle r3 = {wBounds.x + wBounds.width, wBounds.y - depth, depth, wBounds.height + depth};
            const Rectangle r4 = {wBounds.x, wBounds.y + wBounds.height, wBounds.width, depth};
            DrawRectangleLinesEx(r1,2,RED);
            DrawRectangleLinesEx(r2,2,RED);
            DrawRectangleLinesEx(r3,2,RED);
            DrawRectangleLinesEx(r4,2,RED);
        }
        EndMode2D();
    }

    inline void AssignCameraPosition(const entt::registry& registry)
    {
        auto& data = global::ENGINE_DATA;
        const auto view = registry.view<const CameraC, const PositionC>();
        for (const auto e : view)
        {
            const auto pos = view.get<PositionC>(e);
            data.camera.target.x = std::floor(pos.x);
            data.camera.target.y = std::floor(pos.y);
        }
    }

    //----------------- UPDATER -----------------//

    inline void InternalUpdatePre(const entt::registry& registry, Game& game) // Before user space update
    {
        global::CMD_DATA.update();      // First incase needs to block input
        InputSystem(registry);          // Before gametick per contract (scripting system)
        global::PARTICLE_DATA.update(); // Order doesnt matter
        LogicSystem(registry);          // Before gametick cause essential

        // Order doesnt matter
        auto& config = global::ENGINE_CONFIG;
        if (config.showPerformanceOverlay)
        {
            global::PERF_DATA.updateValues();
        }
        if (config.benchmarkTicks > 0) [[unlikely]]
        {
            config.benchmarkTicks--;
            if (config.benchmarkTicks == 0)
                game.shutDown();
        }

        // Before so user can react to changes
        static int achieveCounter = 0;
        if (achieveCounter > 30)
        {
            CheckAchievements();
            achieveCounter = 0;
        }
        ++achieveCounter;
    }

    inline void InternalUpdatePost(const entt::registry& registry) // After user space update
    {
        StaticCollisionSystem();       // Static before cause can cause change in position
        DynamicCollisionSystem();      // After cause user systems can modify entity state
        global::UI_DATA.update();      // After gametick so ui reflects current state
        global::AUDIO_PLAYER.update(); // After game tick cause position updates
    }


} // namespace magique

#endif //RENDERUTIL_H