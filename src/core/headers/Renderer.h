#pragma once

namespace magique::renderer
{
    static double startTime;
    static chrono::time_point<chrono::steady_clock> startTimeH;

    inline void Setup()
    {
        SetupThreadPriority(0); // Thread 0
        SetupProcessPriority();
    }

    inline void Close()
    {
        for (uint_fast32_t i = 1; i < 15000; i++)
        {
            rlUnloadTexture(i);
        }
    }

    inline void StartTick()
    {
        startTime = glfwGetTime();
        startTimeH = chrono::steady_clock::now();
        PollInputEvents();
        BeginDrawing();
        AssignDrawTickCamera();
    }

    inline void EndTick()
    {
        if (global::CONFIGURATION.showPerformanceOverlay)
        {
            global::PERF_DATA.perfOverlay.draw();
        }
        EndDrawing();
        const double frameTime = glfwGetTime() - startTime;
        CORE.Time.frame = frameTime;
        CORE.Time.frameCounter++;
        global::PERF_DATA.saveTickTime(DRAW, (chrono::steady_clock::now() - startTimeH).count());
    }

    inline void RenderTick(bool& isLoading, Game& game, entt::registry& registry, Camera2D& camera)
    {
        StartTick();
        {
            ClearBackground(RAYWHITE); // Thanks ray
            game.preRender();          // Pre render
            if (isLoading) [[unlikely]]
            {
                HandleLoadingScreen(isLoading, game);
                EndTick();
                return;
            }
            BeginMode2D(camera);
            {
                game.drawWorld(camera);
                global::LOGIC_TICK_DATA.lock();
                game.drawGame(registry, camera); // Draw game
                global::LOGIC_TICK_DATA.unlock();
                RenderLighting(registry);
            }
            EndMode2D();
            game.drawUI(); // Draw UI
        }
        EndTick();
    }

    inline void Run(bool& isLoading, Game& game)
    {
        Setup();

        auto& registry = REGISTRY;
        auto& camera = global::DRAW_TICK_DATA.camera;

        // Double loop to catch the close event
        while (game.isRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.isRunning()) [[likely]]
            {
                RenderTick(isLoading, game, registry, camera);
                WaitTime(startTime + CORE.Time.target, CORE.Time.wait - CORE.Time.frame);
            }
            game.onCloseEvent();
        }
    }


} // namespace magique::renderer