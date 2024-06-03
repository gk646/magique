#pragma once

#include "rlgl.h"

#include <magique/ecs/Registry.h>
#include <magique/game/Game.h>

#include "ui/CoreUI.h"
#include "core/CoreData.h"

namespace magique::renderer
{
    using namespace std::chrono;
    inline static time_point<steady_clock> startTime;

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
    }

    inline void StartRenderTick(Game& game)
    {

        startTime = steady_clock::now();
        game.preRender(); // Pre render
    }

    inline void EndRenderTick()
    {
        DrawUI();
        PERF_DATA.saveTickTime(DRAW, (steady_clock::now() - startTime).count());
        EndDrawing();
    }

    inline void Run(bool& isLoading, Game& game)
    {
        auto& reg = ecs::GetRegistry();
        // Double loop to catch the close event
        while (game.isRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.isRunning()) [[likely]]
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