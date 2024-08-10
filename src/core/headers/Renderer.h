#ifndef RENDERER_H
#define RENDERER_H

namespace magique::renderer
{
    inline void Close()
    {
        for (uint_fast32_t i = 1; i < 15000; i++)
        {
            rlUnloadTexture(i);
        }
    }

    inline void StartTick()
    {
        BeginDrawing();
        RLGL.State.prevDrawCalls = RLGL.State.drawCalls;
        RLGL.State.drawCalls = 0;
    }

    inline double EndTick(const double starTime)
    {
        if (global::ENGINE_CONFIG.showPerformanceOverlay)
        {
            global::PERF_DATA.draw();
        }
        EndDrawing();
        const double frameTime = GetTime() - starTime;
        global::PERF_DATA.saveTickTime(DRAW, static_cast<uint32_t>(frameTime * 1'000'000'000.0F));
        return frameTime;
    }

    inline double Tick(const double startTime, Game& game, entt::registry& registry)
    {
        auto& camera = global::ENGINE_DATA.camera;
        StartTick();
        {
            ClearBackground(RAYWHITE); // Thanks ray
            if (game.getIsLoading()) [[unlikely]]
            {
                HandleLoadingScreen(game);
                return EndTick(startTime);
            }
            BeginMode2D(camera);
            {
                game.drawGame(GetGameState()); // Draw game
                if (global::ENGINE_CONFIG.showHitboxes) [[unlikely]]
                    RenderHitboxes(registry);
                RenderLighting(registry);
            }
            EndMode2D();
            global::COMMAND_LINE.draw();
        }
        return EndTick(startTime);
    }

} // namespace magique::renderer
#endif //RENDERER_H