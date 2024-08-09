#ifndef MAINTHREAD_H
#define MAINTHREAD_H

namespace magique::mainthread
{
    inline double UPDATE_WORK = 0.0F;
    inline double PREV_UPDATE_TIME = 0.0F;
    inline double PREV_RENDER_TIME = 0.0F;

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
        auto& config = global::ENGINE_CONFIG.timing;

        // Double loop to catch the close event
        while (game.getIsRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.getIsRunning()) [[likely]]
            {
                auto startTime = GetTime();
                WakeUpJobs();
                if (UPDATE_WORK >= 1.0F)
                {
                    UPDATE_WORK -= 1.0F;
                    PREV_UPDATE_TIME = updater::Tick(startTime, registry, game);
                    startTime += PREV_UPDATE_TIME;
                }
                PREV_RENDER_TIME = renderer::Tick(startTime, game, registry);
                UPDATE_WORK += config.workPerTick;
                config.frameCounter++;

                // Predict next frame time by last time - sleep shorter if next tick a update happens
                const auto nextFrameTime = PREV_RENDER_TIME + (UPDATE_WORK >= 1.0F) * PREV_UPDATE_TIME;
                const auto endTime = startTime + PREV_RENDER_TIME;
                // How much of the time we sleep - round down to nearest millisecond as sleep accuracy is 1ms
                const auto sleepTime = std::floor((config.sleepTime - nextFrameTime) * 1000) / 1000;
                const auto target = endTime + (config.frameTarget - nextFrameTime); // How long we wait in total
                HibernateJobs(target, sleepTime);
                WaitTime(target, sleepTime);
            }

            game.onCloseEvent();
        }
    }

} // namespace magique::mainthread


#endif //MAINTHREAD_H