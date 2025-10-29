// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MAINTHREAD_H
#define MAGIQUE_MAINTHREAD_H

namespace magique::mainthread
{
    inline double UPDATE_WORK = 0.0F;
    inline double UPDATE_TIME = 0.0F;
    inline double RENDER_TIME = 0.0F;

    inline void Setup()
    {
        SetupThreadPriority(0); // Thread 0
        SetupProcessPriority();
    }

    inline void Close()
    {
        renderer::Close();
        global::SCHEDULER.close(); // Needs to be called explicitly so
    }

    inline void Run(Game& game)
    {
        const auto& registry = internal::REGISTRY;
        auto& config = global::ENGINE_CONFIG.timing;
        auto& data = global::ENGINE_DATA;

        // Double loop to catch the close event
        while (game.getIsRunning()) [[likely]]
        {
            while (!WindowShouldClose() && game.getIsRunning()) [[likely]]
            {
                auto time = GetTime();
                data.engineTime = static_cast<float>(time);

                WakeUpJobs();
                global::UI_DATA.updateBeginTick();

                if (UPDATE_WORK >= 1.0)
                {
                    UPDATE_WORK -= 1.0;
                    UPDATE_TIME = updater::Tick(time, game, registry);
                    time += UPDATE_TIME; // Avoids calling GetTime() multiple times
                    ++data.engineTicks;
                }

                // The concept is
                // If the update tick was too fast we want to cleanly wait until the next render tick
                // But I guess this violates our "on average" rule by forcing actual tick times
                // And this cause issues - max framerate is way lower even when the tick times would allow for higher
                // I guess this introduced some blocking and messes with the general strategy
                // WaitTime(NEXT_RENDER, 0); // Dont render too early
                // time = GetTime();

                RENDER_TIME = renderer::Tick(time, game, registry);
                time += RENDER_TIME;

                UPDATE_WORK += config.workPerTick;

                // Predict next frame time by last time - sleep shorter if next tick an update happens
                const auto nextFrameTime = RENDER_TIME + (static_cast<double>(UPDATE_WORK >= 1.0) * UPDATE_TIME);
                // How much of the time we sleep - round down to nearest millisecond as sleep accuracy is 1ms
                const auto sleepTime = std::floor((config.sleepTime - nextFrameTime) * 1000) / 1000;
                const auto target = time + (config.frameTarget - nextFrameTime); // How long we wait in total

                HibernateJobs(target, sleepTime);
                WaitTime(target, sleepTime);
                config.frameCounter++;
            }
            PollInputEvents(); // Somehow needed to prevent crash on wayland
            game.onCloseEvent();
        }
        WakeUpJobs(); // To finish all saving tasks
    }

} // namespace magique::mainthread


#endif //MAGIQUE_MAINTHREAD_H
