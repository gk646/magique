// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_RENDERER_H
#define MAGIQUE_RENDERER_H

namespace magique::renderer
{
    inline static constexpr int BIG_NUMBER = 50'000;

    inline void Close()
    {
        for (uint_fast32_t i = 1; i < BIG_NUMBER; i++)
        {
            rlUnloadTexture(i);
        }
    }

    inline void StartTick()
    {
        AssignCameraPosition();
        ResetDrawCallCount();
    }

    inline double EndTick(const double startTime)
    {
        const auto& cmdData = global::CONSOLE_DATA;
        auto& perfData = global::PERF_DATA;
        perfData.draw();
        cmdData.draw();

        EndDrawing();
        SwapScreenBuffer();
        return global::PERF_DATA.drawTick.add(GetTime() - startTime);
    }

    inline double Tick(const double startTime, Game& game, const entt::registry& registry)
    {
        const auto gameState = global::ENGINE_DATA.gameState;
        StartTick();
        {
            ClearBackground(Color{163, 163, 163, 255});
            if (game.getIsLoading()) [[unlikely]]
            {
                HandleLoadingScreen(game); // Loading screen
                return EndTick(startTime);
            }
            game.onDrawGame(gameState, CameraGet()); // User draw tick
            InternalRenderPost();                    // Post user tick render tasks
            game.onDrawUI(gameState);                // User UI raw tick
        }
        return EndTick(startTime);
    }

} // namespace magique::renderer

#endif // MAGIQUE_RENDERER_H
