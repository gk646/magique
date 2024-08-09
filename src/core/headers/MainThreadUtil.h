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
    const int ret = static_cast<int>(std::round(static_cast<float>(sumFPS) / count));

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
            game.drawLoadingScreen(GetUIRoot(), loader->getProgressPercent());
            const auto res = loader->step();
            if (res)
            {
                ResetBenchmarkTimes();
                delete loader;
                loader = nullptr;
                game.isLoading = false;
            }
        }
    }

    inline void RenderHitboxes(const entt::registry& reg)
    {
        for (const auto e : GetDrawEntities())
        {
            const auto& pos = reg.get<const PositionC>(e);
            const auto& col = reg.get<const CollisionC>(e);
            switch (col.shape)
            {
            [[likely]] case Shape::RECT:
                DrawRectangleLinesRot({pos.x, pos.y, col.p1, col.p2}, pos.rotation, col.anchorX, col.anchorY, RED);
                break;
            case Shape::CIRCLE:
                DrawCircleLinesV({pos.x + col.p1 / 2.0F, pos.y + col.p1 / 2.0F}, col.p1, RED);
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
    }

    //----------------- UPDATER -----------------//

    inline void InternalUpdate(const entt::registry& registry)
    {
        global::COMMAND_LINE.update(); // First incase needs to block input
        InputSystem(registry);
        global::UI_DATA.update();
        global::AUDIO_PLAYER.update();
        global::PARTICLE_DATA.update();
        LogicSystem(registry);
        CollisionSystem(registry);
    }

} // namespace magique

#endif //RENDERUTIL_H