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
        const auto view = reg.view<const PositionC, const CollisionC>();
        for (const auto e : view)
        {
            const auto& pos = view.get<const PositionC>(e);
            const auto& col = view.get<const CollisionC>(e);
            switch (col.shape)
            {
            [[likely]] case Shape::RECT:
                DrawRectangleLinesRot({pos.x, pos.y, col.p1, col.p2}, pos.rotation, col.anchorX, col.anchorY, RED);
                break;
            case Shape::CIRCLE:
                DrawCircleLinesV({pos.x + col.p1 / 2.0F, pos.y + col.p1 / 2.0F}, col.p1, RED);
                break;
            case Shape::CAPSULE:
                DrawCapsule2DLines(pos.x, pos.y, col.p1, col.p2, RED);
                break;
            case Shape::TRIANGLE:
                DrawTriangleLinesRot({pos.x, pos.y}, {pos.x + col.p1, pos.y + col.p2}, {pos.x + col.p3, pos.y + col.p4},
                                     pos.rotation, col.anchorX, col.anchorY, RED);
                break;
            }
        }
    }

    //----------------- UPDATER -----------------//

    inline void InternalUpdate(entt::registry& registry)
    {
        global::UI_DATA.update();
        global::AUDIO_PLAYER.update();
        global::COMMAND_LINE.update();
        InputSystem(registry);
        LogicSystem(registry);
        CollisionSystem(registry);
    }

} // namespace magique

#endif //RENDERUTIL_H