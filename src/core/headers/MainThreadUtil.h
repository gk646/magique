#ifndef MAGIQUE_MAIN_THREAD_UTIL_H
#define MAGIQUE_MAIN_THREAD_UTIL_H

inline void SetTargetFPS(const int fps) // raylib function implemented here
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

inline int GetFPS()
{
    auto& config = magique::global::ENGINE_CONFIG.timing;
#define FPS_BUFF_SIZE 15
    static double lastTime = 0;
    static int fpsBuffer[FPS_BUFF_SIZE] = {0};
    static int index = 0;
    static int sumFPS = 0;
    static float count = 0;

    const double currentTime = GetTime();

    const int currFPS =
        static_cast<int>(std::round(static_cast<double>(config.frameCounter) / (currentTime - lastTime)));

    sumFPS -= fpsBuffer[index];
    fpsBuffer[index] = currFPS;
    sumFPS += currFPS;
    index = (index + 1) % FPS_BUFF_SIZE;

    if (count < FPS_BUFF_SIZE)
    {
        count++;
    }
    const int ret = static_cast<int>(std::round(static_cast<float>(sumFPS) / count));

    lastTime = currentTime;
    config.frameCounter = 0;

    return ret;
}

// Get time in seconds for last frame drawn (delta time)
inline float GetFrameTime()
{
    const auto& perf = magique::global::PERF_DATA;
    return static_cast<float>(perf.drawTickTime + perf.logicTickTime) / 1'000'000'000.0F;
}

namespace magique
{
    //----------------- RENDERER -----------------//

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
        const auto& staticData = global::STATIC_COLL_DATA;

        const auto bounds = GetCameraBounds();
        const auto map = GetCameraMap();

        // Dynamic entities
        for (const auto e : GetDrawEntities())
        {
            if (!group.contains(e))
                continue;

            const auto& pos = group.get<const PositionC>(e);
            const auto& col = group.get<const CollisionC>(e);

            if (!PointToRect(pos.x, pos.y, bounds.x, bounds.y, bounds.width, bounds.height))
                continue;

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

        //TODO clean up
        // Static tile map objects
        if (staticData.objectReferences.tileObjectMap.contains(map))
        {
            for (const auto idx : staticData.objectReferences.tileObjectMap.at(map))
            {
                const auto& [x, y, p1, p2] = staticData.objectStorage.get(idx);
                if (p1 == 0 || !PointToRect(x, y, bounds.x, bounds.y, bounds.width, bounds.height))
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
        }

        // Static tiles
        if (staticData.objectReferences.tilesDataMap.contains(map))
        {
            for (const auto idx : staticData.objectReferences.tilesDataMap.at(map))
            {
                const auto& [x, y, p1, p2] = staticData.objectStorage.get(idx);
                if (p1 == 0 || !PointToRect(x, y, bounds.x, bounds.y, bounds.width, bounds.height))
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
        }


        if (staticData.getIsWorldBoundSet()) // enabled
        {
            constexpr float depth = MAGIQUE_WORLD_BOUND_DEPTH;
            const auto wBounds = staticData.worldBounds;
            const Rectangle r1 = {wBounds.x - depth, wBounds.y - depth, depth, wBounds.height + depth};
            const Rectangle r2 = {wBounds.x, wBounds.y - depth, wBounds.width, depth};
            const Rectangle r3 = {wBounds.x + wBounds.width, wBounds.y - depth, depth, wBounds.height + depth};
            const Rectangle r4 = {wBounds.x, wBounds.y + wBounds.height, wBounds.width, depth};
            DrawRectangleLinesEx(r1, 2, RED);
            DrawRectangleLinesEx(r2, 2, RED);
            DrawRectangleLinesEx(r3, 2, RED);
            DrawRectangleLinesEx(r4, 2, RED);
        }
        EndMode2D();
    }

    inline void AssignCameraPosition(const entt::registry& registry)
    {
        auto& data = global::ENGINE_DATA;
        auto& config = global::ENGINE_CONFIG;
        const auto view = registry.view<const CameraC, const PositionC>();
        const auto smoothing = 1.0F - config.cameraSmoothing; // The higher the value the smoother
        for (const auto e : view)
        {
            const auto& pos = view.get<PositionC>(e);
            const auto coll = internal::REGISTRY.try_get<CollisionC>(e);

            Point targetPosition{pos.x, pos.y};
            if (coll) [[likely]]
            {
                switch (coll->shape)
                {
                case Shape::RECT:
                    targetPosition.x += coll->p1 / 2.0F;
                    targetPosition.y += coll->p2 / 2.0F;
                    break;
                case Shape::CIRCLE:
                    targetPosition.x += coll->p1;
                    targetPosition.y += coll->p1;
                    break;
                case Shape::CAPSULE:
                    targetPosition.x += coll->p1;
                    targetPosition.y += coll->p2 / 2.0F;
                    break;
                case Shape::TRIANGLE:
                    break;
                }
            }
            data.camera.target.x =
                std::floor(data.camera.target.x + (targetPosition.x - data.camera.target.x) * smoothing);
            data.camera.target.y =
                std::floor(data.camera.target.y + (targetPosition.y - data.camera.target.y) * smoothing);
        }
    }

    //----------------- UPDATER -----------------//

    inline void InternalUpdatePre(const entt::registry& registry, Game& game) // Before user space update
    {
        global::CMD_DATA.update();      // First in case needs to block input
        InputSystem(registry);          // Before gametick per contract (scripting system)
        global::PARTICLE_DATA.update(); // Order doesnt matter
        LogicSystem(registry);          // Before gametick cause essential
                                        //        global::PATH_DATA.updateDynamicGrid();

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

        // Before, so user can react to changes
        static int achieveCounter = 0;
        if (achieveCounter > 30)
        {
            CheckAchievements();
            achieveCounter = 0;
        }
        ++achieveCounter;

        // Reset nearby query
        global::ENGINE_DATA.nearbyQueryData.lastRadius = 0;
    }

    inline void InternalUpdatePost() // After user space update
    {
        StaticCollisionSystem();       // After cause user systems can modify entity state
        DynamicCollisionSystem();      // After cause user systems can modify entity state
        global::UI_DATA.update();      // After gametick so ui reflects current state
        global::AUDIO_PLAYER.update(); // After game tick cause position updates
    }

} // namespace magique

#endif //MAGIQUE_MAIN_THREAD_UTIL_H