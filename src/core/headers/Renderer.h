
namespace magique::renderer
{
    static double startTime;
    static chrono::time_point<chrono::steady_clock> startTimeH;

    void StartRenderTick()
    {
        startTime = glfwGetTime();
        startTimeH = chrono::steady_clock::now();
        PollInputEvents();
        BeginDrawing();
        AssignDrawTickCamera();
    }

    void EndRenderTick()
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

    void HandleLoadingScreen(bool& isLoading, Game& game)
    {
        auto& loader = global::CURRENT_GAME_LOADER;
        if (loader) [[likely]]
        {
            game.drawLoadingScreen(loader->getProgressPercent());
            const auto res = loader->step();
            EndRenderTick();
            if (res == true)
            {
                global::PERF_DATA.drawTimes.clear();
                global::PERF_DATA.logicTimes.clear();
                delete loader;
                loader = nullptr;
                isLoading = false;
            }
        }
    }

    void RenderTick(bool& isLoading, Game& game, entt::registry& registry, Camera2D& camera)
    {
        StartRenderTick();
        {
            ClearBackground(RAYWHITE); // Thanks ray
            game.preRender();          // Pre render
            if (isLoading) [[unlikely]]
            {
                HandleLoadingScreen(isLoading, game);
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
        EndRenderTick();
    }

    void Setup()
    {
#if defined(WIN32)
        HANDLE hThread = GetCurrentThread();
        SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
        HANDLE hProcess = GetCurrentProcess();
        SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);
        DWORD_PTR processAffinityMask = 0xF;
        if (!SetProcessAffinityMask(hProcess, processAffinityMask)) {
            fprintf(stderr, "Failed to set process affinity mask\n");
        }
        DWORD_PTR affinityMask = 1;
        auto res = SetThreadAffinityMask(hThread, affinityMask);
        res = SetThreadAffinityMask(hThread, affinityMask);
        if (res == 0) {
            fprintf(stderr, "Failed to set thread affinity mask\n");
        }
#endif
    }

    void Run(bool& isLoading, Game& game)
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

    void Close()
    {
        for (uint_fast32_t i = 1; i < 15000; i++)
        {
            rlUnloadTexture(i);
        }
    }

} // namespace magique::renderer