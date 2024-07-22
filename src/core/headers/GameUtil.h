#ifndef RENDERUTIL_H
#define RENDERUTIL_H

namespace magique
{
    //----------------- RENDERER -----------------//
    // Logic loop as authority over the state
    // Render thread just draws the current state
    inline void AssignDrawTickCamera()
    {
        global::LOGIC_TICK_DATA.lock();
        {
            auto& drawTick = global::DRAW_TICK_DATA;
            auto& logicTick = global::LOGIC_TICK_DATA;

            drawTick.camera = logicTick.camera;
            drawTick.cameraMap = logicTick.cameraMap;
            drawTick.cameraEntity = logicTick.cameraEntity;
        }
        global::LOGIC_TICK_DATA.unlock();
    }

    inline void HandleLoadingScreen(bool& isLoading, Game& game)
    {
        auto& loader = global::CURRENT_GAME_LOADER;
        if (loader) [[likely]]
        {
            game.drawLoadingScreen(loader->getProgressPercent());
            const auto res = loader->step();
            if (res == true)
            {
                global::PERF_DATA.drawTimes.clear();
                global::PERF_DATA.logicTimes.clear();
                delete loader;
                loader = nullptr;
                isLoading = false;
            }
        }
    }

    inline void RenderHitboxes(const entt::registry& reg)
    {
        const auto view = reg.view<PositionC, CollisionC>();
        for (const auto e : view)
        {
            auto& pos = view.get<PositionC>(e);
            auto& col = view.get<CollisionC>(e);

            switch (col.shape)
            {
            case CIRCLE:
                DrawCircleLinesV({pos.x + col.width / 2.0F, pos.y + col.height / 2.0F}, col.width, RED);
                break;
            case RECT:
                DrawRectangleLinesEx({pos.x, pos.y, (float)col.width, (float)col.height},2.0F,RED);
                break;
            case POLYGON:
                break;
            }
        }
    }

    //----------------- UPDATER -----------------//

    inline void InternalUpdate(entt::registry& registry)
    {
        global::AUDIO_PLAYER.update();
        PollInputs(registry);
        UpdateLogic(registry);
        CheckCollisions(registry);
    }

} // namespace magique

#endif //RENDERUTIL_H