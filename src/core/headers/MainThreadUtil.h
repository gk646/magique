#ifndef RENDERUTIL_H
#define RENDERUTIL_H

inline double FRAME_TARGET = 0.0F;
inline double WAIT_TIME = 0.0F;
inline double START_TIME = 0.0F;
inline double PREV_UPDATE_TIME = 0.0F;
inline double PREV_RENDER_TIME = 0.0F;
inline double UPDATE_WORK = 0.0F;
inline double WORK_PER_TICK = 0.0F;

void SetTargetFPS(const int fps) // raylib function implemented here
{
    if (fps < 1)
        FRAME_TARGET = 0;
    else
    {
        FRAME_TARGET = 1.0 / static_cast<double>(fps);
        // Round down cause minimal accuracy is only 1ms - so wait 1 ms less to be accurate
        WAIT_TIME = std::floor((FRAME_TARGET * 1000) * 0.95F);
        WORK_PER_TICK = MAGIQUE_LOGIC_TICKS / static_cast<double>(fps);
    }
}

int GetFPS(void)
{
#define FPS_BUFF_SIZE 15
    static float lastTime = 0;
    static int fpsBuffer[FPS_BUFF_SIZE] = {0};
    static int index = 0;
    static int sumFPS = 0;
    static int count = 0;

    const double currentTime = glfwGetTime();

    const int currFPS = CORE.Time.frameCounter / (currentTime - lastTime);

    sumFPS -= fpsBuffer[index];
    fpsBuffer[index] = currFPS;
    sumFPS += currFPS;
    index = (index + 1) % FPS_BUFF_SIZE;

    if (count < FPS_BUFF_SIZE)
    {
        count++;
    }

    const int ret = (int)ceil((float)sumFPS / count);

    lastTime = currentTime;
    CORE.Time.frameCounter = 0;

    return ret;
}

// Get time in seconds for last frame drawn (delta time)
float GetFrameTime(void)
{
    return (float)CORE.Time.frame / 1'000'000'000.0F;
}

namespace magique
{
    //----------------- RENDERER -----------------//

    // Logic loop as authority over the state
    // Render thread just draws the current state
    inline void AssignDrawTickCamera()
    {
        global::LOGIC_TICK_DATA.lock();
        {
            auto& drawTick = global::DRAW_TICK_DATA;
            auto& logicTick = global::LOGIC_TICK_DATA;

            drawTick.camera = logicTick.camera;
            drawTick.cameraMap = logicTick.cameraMap;
            drawTick.cameraEntity = logicTick.cameraEntity;
        }
        global::LOGIC_TICK_DATA.unlock();
    }

    inline void HandleLoadingScreen(Game& game)
    {
        auto& loader = global::LOADER;
        if (loader != nullptr) [[likely]]
        {
            game.drawLoadingScreen(GetUIRoot(), loader->getProgressPercent());
            const auto res = loader->step();
            if (res)
            {
                global::PERF_DATA.drawTimes.clear();
                global::PERF_DATA.logicTimes.clear();
                delete loader;
                loader = nullptr;
                game.isLoading = false;
            }
        }
    }

    inline void RenderHitboxes(const entt::registry& reg)
    {
        const auto view = reg.view<const PositionC, const CollisionC>();
        for (const auto e : view)
        {
            const auto& pos = view.get<const PositionC>(e);
            const auto& col = view.get<const CollisionC>(e);
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

    inline void InternalUpdate(entt::registry& registry)
    {
        global::UI_DATA.update();
        global::AUDIO_PLAYER.update();
        global::COMMAND_LINE.update();
        global::PARTICLE_DATA.update();
        InputSystem(registry);
        LogicSystem(registry);
        CollisionSystem(registry);
    }

} // namespace magique

#endif //RENDERUTIL_H