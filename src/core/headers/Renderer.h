#ifndef RENDERER_H
#define RENDERER_H

namespace magique::renderer
{
    inline void Close()
    {
        for (uint_fast32_t i = 1; i < 50'000; i++)
        {
            rlUnloadTexture(i);
        }
    }

    inline void StartTick(const entt::registry& registry)
    {
        auto& rlglState = RLGL.State;
        BeginDrawing();
        rlglState.prevDrawCalls = rlglState.drawCalls;
        rlglState.drawCalls = 0;
        AssignCameraPosition(registry);
        global::UI_DATA.updateDrawTick();
    }

    inline double EndTick(const double starTime)
    {
        const auto& config = global::ENGINE_CONFIG;
        auto& perfData = global::PERF_DATA;
        global::UI_DATA.inputConsumed = false; // End of render ticks vs start of update tick is the same
        if (config.showPerformanceOverlay)
        {
            perfData.draw();
        }
        EndDrawing();
        const double frameTime = GetTime() - starTime;
        perfData.saveTickTime(DRAW, static_cast<uint32_t>(frameTime * 1'000'000'000.0F));
        return frameTime;
    }

    inline double Tick(const double startTime, Game& game, const entt::registry& registry)
    {
        const auto& config = global::ENGINE_CONFIG;
        auto& data = global::ENGINE_DATA;
        auto& cmdData = global::CMD_DATA;

        auto& camera = data.camera;
        const auto gameState = GetGameState();
        StartTick(registry);
        {
            ClearBackground(RAYWHITE); // Thanks ray
            if (game.getIsLoading()) [[unlikely]]
            {
                HandleLoadingScreen(game); // Loading screen
                return EndTick(startTime);
            }
            game.drawGame(gameState, camera); // Draw game
            if (config.showHitboxes) [[unlikely]]
                RenderHitboxes();
            RenderLighting(registry);
            game.drawUI(gameState);
            //uiData.draw();
            cmdData.draw();
        }
        return EndTick(startTime);
    }

} // namespace magique::renderer
#endif //RENDERER_H