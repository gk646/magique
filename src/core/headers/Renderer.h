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

    inline void StartTick(const entt::registry& registry)
    {
        BeginDrawing();
        RLGL.State.prevDrawCalls = RLGL.State.drawCalls;
        RLGL.State.drawCalls = 0;
        const auto view = registry.view<const CameraC, const PositionC>();
        for (const auto e : view)
        {
            const auto pos = view.get<PositionC>(e);
            global::ENGINE_DATA.camera.target = {pos.x, pos.y};
        }
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

    inline double Tick(const double startTime, Game& game, const entt::registry& registry)
    {
        auto& camera = global::ENGINE_DATA.camera;
        const auto gameState = GetGameState();
        StartTick(registry);
        {
            ClearBackground(RAYWHITE); // Thanks ray
            if (game.getIsLoading()) [[unlikely]]
            {
                HandleLoadingScreen(game);
                return EndTick(startTime);
            }
            game.drawGame(gameState, camera); // Draw game
            if (global::ENGINE_CONFIG.showHitboxes) [[unlikely]]
                RenderHitboxes(registry);
            RenderLighting(registry);

            global::UI_DATA.clearRenderObjects();
            game.drawUI(gameState);
            global::UI_DATA.draw();

            global::COMMAND_LINE.draw();
        }
        return EndTick(startTime);
    }

} // namespace magique::renderer
#endif //RENDERER_H