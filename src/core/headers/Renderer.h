#ifndef MAGIQUE_RENDERER_H
#define MAGIQUE_RENDERER_H

namespace magique::renderer
{
    inline static constexpr int BIG_NUMBER = 50'0000;

    inline void Close()
    {
        for (uint_fast32_t i = 1; i < BIG_NUMBER; i++)
        {
            rlUnloadTexture(i);
        }
    }

    inline void StartTick()
    {
        rlLoadIdentity();
        rlMultMatrixf(MatrixToFloat(GetScreenScale()));

        ResetDrawCallCount();
        AssignCameraPosition();
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
        SwapScreenBuffer();
        const double frameTime = GetTime() - starTime;
        perfData.saveTickTime(DRAW, static_cast<uint32_t>(frameTime * SEC_TO_NANOS));
        return frameTime;
    }

    inline double Tick(const double startTime, Game& game, const entt::registry& registry)
    {
        const auto& config = global::ENGINE_CONFIG;
        auto& data = global::ENGINE_DATA;
        auto& cmdData = global::CONSOLE_DATA;

        auto& camera = data.camera;
        const auto gameState = GetGameState();
        StartTick();
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
            cmdData.draw();
        }
        return EndTick(startTime);
    }

} // namespace magique::renderer

#endif //MAGIQUE_RENDERER_H