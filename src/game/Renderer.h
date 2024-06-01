#pragma once

#include "rlgl.h"

#include <magique/ecs/Registry.h>
#include <magique/game/Game.h>

#include "core/CoreData.h"

namespace magique::renderer
{
    using namespace std::chrono;
    inline static time_point<steady_clock> starTime;


    inline void HandleLoadingScreen(bool& isLoading, Game& game)
    {
        if (CURRENT_GAME_LOADER)
        {
            const auto res = CURRENT_GAME_LOADER->load();
            game.drawLoadingScreen(CURRENT_GAME_LOADER->getProgressPercent());
            if (res == true)
            {
                delete CURRENT_GAME_LOADER;
                CURRENT_GAME_LOADER = nullptr;
                isLoading = false;
            }
        }
    }

    inline void StartRenderTick(Game& game)
    {
        starTime = steady_clock::now();
        game.preRender(); // Pre render
    }

    inline void EndRenderTick()
    {
        EndDrawing();
        PERF_DATA.saveTickTime(DRAW, (steady_clock::now() - starTime).count());
    }

    inline void Run(const bool& isRunning, bool& isLoading, Game& game)
    {
        auto& reg = ecs::GetRegistry();
        // Double loop to catch the close event
        while (isRunning) [[likely]]
        {
            while (!WindowShouldClose() && isRunning) [[likely]]
            {
                StartRenderTick(game);
                BeginDrawing();
                {
                    ClearBackground(RAYWHITE); // Thanks ray
                    if (isLoading) [[unlikely]]
                    {
                        HandleLoadingScreen(isLoading, game);
                    }

                    auto& camera = game.camera;
                    BeginMode2D(camera);
                    {
                        game.drawGame(reg, camera); // Draw game
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
        for (uint_fast32_t i = 1; i < 10000; i++)
        {
            rlUnloadTexture(i);
        }
    }


} // namespace magique::renderer