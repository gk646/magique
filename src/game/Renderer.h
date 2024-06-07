#pragma once

#include "rlgl.h"

#include <magique/ecs/ECS.h>
#include <magique/core/Game.h>

#include "ui/CoreUI.h"
#include "ecs/systems/LightingSystem.h"

namespace magique::renderer
{
    using namespace std::chrono;
    inline static time_point<steady_clock> startTime;

    inline void StartRenderTick()
    {
        startTime = steady_clock::now();
        BeginDrawing();
    }

    inline void EndRenderTick()
    {
        DrawUI();
        PERF_DATA.saveTickTime(DRAW, (steady_clock::now() - startTime).count());
        EndDrawing();
    }

    inline void HandleLoadingScreen(bool& isLoading, Game& game)
    {
        if (CURRENT_GAME_LOADER) [[likely]]
        {
            game.drawLoadingScreen(CURRENT_GAME_LOADER->getProgressPercent());
            const auto res = CURRENT_GAME_LOADER->load();
            if (res == true)
            {
                delete CURRENT_GAME_LOADER;
                CURRENT_GAME_LOADER = nullptr;
                isLoading = false;
            }
        }
        EndRenderTick();
    }

    inline void Run(bool& isLoading, Game& game)
    {
        auto& reg = REGISTRY;
        auto& camera = LOGIC_TICK_DATA.camera;
        // Double loop to catch the close event
        while (game.isRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.isRunning()) [[likely]]
            {
                StartRenderTick();
                {
                    ClearBackground(RAYWHITE); // Thanks ray
                    game.preRender(); // Pre render
                    if (isLoading) [[unlikely]]
                    {
                        HandleLoadingScreen(isLoading, game);
                        continue;
                    }
                    BeginMode2D(camera);
                    {
                        game.drawWorld(camera);
                        LOGIC_TICK_DATA.lock();
                        game.drawGame(reg, camera); // Draw game
                        LOGIC_TICK_DATA.unlock();
                        RenderLighting(reg);
                    }
                    EndMode2D();
                    game.drawUI(); // Draw UI
                }
                EndRenderTick();
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