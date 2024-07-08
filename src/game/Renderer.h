#pragma once

#include "game/common/RenderUtil.h"
#include "ecs/systems/LightingSystem.h"

#if defined(_WIN32)
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processthreadsapi.h>
#endif

namespace magique::renderer
{
    using namespace std::chrono;
    inline static time_point<steady_clock> startTime;

    inline void WaitTimeM()
    {
        const auto targetFrameTime = nanoseconds(CORE.Time.target);
        if (CORE.Time.frame < CORE.Time.target)
        {
            const auto sleepNanos = CORE.Time.wait - CORE.Time.frame;
#if defined(_WIN32)
                Sleep((unsigned long)(sleepNanos / 1'000'000.0F));
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
            // Use nanosleep on Unix-like platforms
            struct timespec req;
            req.tv_sec = sleepNanos / 1'000'000'000;  // Convert nanoseconds to seconds
            req.tv_nsec = sleepNanos % 1'000'000'000; // Remainder is nanoseconds

            // NOTE: Use nanosleep() on Unix platforms... usleep() is deprecated
            while (nanosleep(&req, &req) == -1)
                continue;
#endif
#if defined(__APPLE__)
            usleep(sleepNanos / 1'000.0);
#endif
            while (steady_clock::now() - startTime < targetFrameTime)
            {
                Sleep(0);
            }
        }
    }

    inline void StartRenderTick()
    {
        startTime = GetTime();
        PollInputEvents();
        BeginDrawing();
        AssignDrawTickCamera();
    }

    inline void EndRenderTick()
    {
        if (global::CONFIGURATION.showPerformanceOverlay)
        {
            global::PERF_DATA.perfOverlay.draw();
        }
        EndDrawing();
        const auto frameTime = static_cast<uint32_t>((steady_clock::now() - startTime).count());
        CORE.Time.frame = frameTime;
        CORE.Time.frameCounter++;
        global::PERF_DATA.saveTickTime(DRAW, frameTime);

    }

    inline void HandleLoadingScreen(bool& isLoading, Game& game)
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

    inline void Run(bool& isLoading, Game& game)
    {
        auto& registry = REGISTRY;
        auto& camera = global::DRAW_TICK_DATA.camera;

        // Double loop to catch the close event
        while (game.isRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.isRunning()) [[likely]]
            {
                StartRenderTick();
                {
                    ClearBackground(RAYWHITE); // Thanks ray
                    game.preRender();          // Pre render
                    if (isLoading) [[unlikely]]
                    {
                        HandleLoadingScreen(isLoading, game);
                        continue;
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
                WaitTimeM();
            }
            game.onCloseEvent();
        }
    }

    inline void Close()
    {
        for (uint_fast32_t i = 1; i < 15000; i++)
        {
            rlUnloadTexture(i);
        }
    }

} // namespace magique::renderer