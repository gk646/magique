// SPDX-License-Identifier: zlib-acknowledgement
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
        AssignCameraPosition();
        ResetDrawCallCount();
    }

    inline double EndTick(const double starTime)
    {
        const auto& cmdData = global::CONSOLE_DATA;
        auto& perfData = global::PERF_DATA;
        perfData.draw();
        cmdData.draw();

        EndDrawing();
        SwapScreenBuffer();
        const double frameTime = GetTime() - starTime;
        perfData.saveTickTime(DRAW, static_cast<uint32_t>(frameTime * SEC_TO_NANOS));
        return frameTime;
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
            game.drawGame(gameState, GetCamera()); // User draw tick
            InternalRenderPost();                  // Post user tick render tasks
            game.drawUI(gameState);                // User UI raw tick
        }
        return EndTick(startTime);
    }

} // namespace magique::renderer

#endif //MAGIQUE_RENDERER_H