#pragma once

#include "game/common/RenderUtil.h"
#include "ecs/systems/LightingSystem.h"
#include "ui/internal/UI.h"

namespace magique::renderer
{
    using namespace std::chrono;
    inline static time_point<steady_clock> startTime;


    inline void StartRenderTick(entt::registry& registry)
    {
        startTime = steady_clock::now();
        BeginDrawing();
        AssignCameraData(registry);
    }

    inline void EndRenderTick()
    {
        DrawUI();
        EndDrawing();
        global::PERF_DATA.saveTickTime(DRAW, (steady_clock::now() - startTime).count());
    }

    inline void HandleLoadingScreen(bool& isLoading, Game& game)
    {
        auto& loader = global::CURRENT_GAME_LOADER;
        if (loader) [[likely]]
        {
            game.drawLoadingScreen(loader->getProgressPercent());
            const auto res = loader->load();
            if (res == true)
            {
                delete loader;
                loader = nullptr;
                isLoading = false;
            }
        }
        EndRenderTick();
    }

    inline void Run(bool& isLoading, Game& game)
    {
        auto& reg = REGISTRY;
        auto& camera = global::DRAW_TICK_DATA.camera;
        // Double loop to catch the close event
        while (game.isRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.isRunning()) [[likely]]
            {
                StartRenderTick(reg);
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
                        game.drawGame(reg, camera); // Draw game
                        global::LOGIC_TICK_DATA.unlock();
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