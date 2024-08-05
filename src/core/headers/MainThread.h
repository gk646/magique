#ifndef MAINTHREAD_H
#define MAINTHREAD_H

namespace magique
{
    inline void Setup()
    {
        SetupThreadPriority(0); // Thread 0
        SetupProcessPriority();
    }

    inline void Close() { renderer::Close(); }

    inline void Run(Game& game)
    {
        auto& registry = internal::REGISTRY;
        auto& camera = global::DRAW_TICK_DATA.camera;

        // Double loop to catch the close event
        while (game.getIsRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.getIsRunning()) [[likely]]
            {
                WakeUpJobs();
                if (UPDATE_WORK >= 1.0F)
                {
                    UPDATE_WORK -= 1.0F;
                    PREV_UPDATE_TIME = updater::Tick(registry, game);
                }
                UPDATE_WORK += WORK_PER_TICK;

                PREV_RENDER_TIME = renderer::Tick(game, registry, camera);


                const auto frameTime = FRAME_TARGET - (PREV_RENDER_TIME + (UPDATE_WORK >= 1.0F) * PREV_UPDATE_TIME);
                const auto endTime = glfwGetTime();
                const auto waitTime = WAIT_TIME - frameTime;
                HibernateJobs(endTime, waitTime);
                WaitTime(glfwGetTime() + (FRAME_TARGET - frameTime), waitTime);
            }
            game.onCloseEvent();
        }
    }


} // namespace magique


#endif //MAINTHREAD_H