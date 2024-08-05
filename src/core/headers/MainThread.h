#ifndef MAINTHREAD_H
#define MAINTHREAD_H
#include <cxutil/cxtime.h>

namespace magique::mainthread
{
    inline void Setup()
    {
        SetupThreadPriority(0); // Thread 0
        SetupProcessPriority();
    }

    inline void Close()
    {
        renderer::Close();
        global::SCHEDULER.close();
    }

    inline void Run(Game& game)
    {
        auto& registry = internal::REGISTRY;

        // Double loop to catch the close event
        while (game.getIsRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.getIsRunning()) [[likely]]
            {
                auto startTime = glfwGetTime();
                WakeUpJobs();
                if (UPDATE_WORK >= 1.0F)
                {
                    UPDATE_WORK -= 1.0F;
                    PREV_UPDATE_TIME = updater::Tick(startTime, registry, game);
                    startTime += PREV_UPDATE_TIME;
                }
                PREV_RENDER_TIME = renderer::Tick(startTime, game, registry);
                UPDATE_WORK += WORK_PER_TICK;
                FRAME_COUNT++;

                // Predict next frame time by last time - sleep shorter if next tick a update happens
                const auto nextFrameTime = PREV_RENDER_TIME + (UPDATE_WORK >= 1.0F) * PREV_UPDATE_TIME;
                const auto endTime = startTime + PREV_RENDER_TIME;
                // How much of the time we sleep - round down to nearest millisecond as sleep accuracy is 1ms
                const auto sleepTime = SLEEP_TIME - nextFrameTime;
                const auto target = endTime + (FRAME_TARGET - nextFrameTime); // How long we wait in total
                HibernateJobs(target, sleepTime);
                WaitTime(target, sleepTime);
            }

            game.onCloseEvent();
        }
    }

} // namespace magique::mainthread


#endif //MAINTHREAD_H