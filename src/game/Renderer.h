#pragma once

#include "rlgl.h"

#include <magique/ecs/Registry.h>
#include <magique/game/Game.h>

namespace magique::renderer
{
    inline void Run(const bool& isRunning, Game& game)
    {
        auto& reg = ecs::GetRegistry();
        // Double loop to catch the close event
        while (isRunning) [[likely]]
        {
            while (!WindowShouldClose() && isRunning) [[likely]]
            {
                const auto startTime = std::chrono::steady_clock::now();
                game.preRender(); // Pre render
                BeginDrawing();
                {
                    ClearBackground(RAYWHITE); // Thanks ray
                    auto& camera = game.camera;
                    BeginMode2D(camera);
                    {
                        game.drawGame(reg, camera); // Draw game
                    }
                    EndMode2D();
                    game.drawUI(); // Draw UI
                }
                EndDrawing();
                const auto tickTime = std::chrono::steady_clock::now() - startTime;
                // Glob::DTD.lastTickTime = tickTime.count();
#ifdef MAGIQUE_DEBUG
                //Glob::GDT.drawTimes.push_back(Glob::DTD.lastTickTime);
#endif
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