#pragma once

namespace magique::updater
{
    constexpr double tickDuration = 1.0F / MAGIQUE_LOGIC_TICKS;
    constexpr double wait = tickDuration * 0.9F;
    static double startTime;
    static double tick;

    inline void Setup()
    {
        SetupThreadPriority(1); // Thread 1
    }

    inline void Close()
    {
        if (global::LOGIC_THREAD.joinable())
        {
            global::LOGIC_THREAD.join();
        }
    }

    inline void StartTick()
    {
        startTime = glfwGetTime();
        WakeUpJobs();
    }

    inline void EndTick(Game& game)
    {
        auto& config = global::CONFIGURATION;
        if (config.showPerformanceOverlay)
        {
            global::PERF_DATA.perfOverlay.updateValues();
        }
        if (config.benchmarkTicks > 0) [[unlikely]]
        {
            config.benchmarkTicks--;
            if (config.benchmarkTicks == 0)
                game.shutDown();
        }
        TickInputEvents();
        const double tickTime = glfwGetTime() - startTime;
        tick = tickTime;
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>(tickTime * 1'000'000'000.0F));
        HibernateJobs(startTime, tickTime);
    }

    inline void GameLoop(const bool& isRunning, Game& game)
    {
        Setup();
        auto& reg = internal::REGISTRY;
        while (isRunning) [[likely]]
        {
            StartTick();
            //Tick game
            {
                InternalUpdate(reg); // Internal update upfront
                game.updateGame(reg);
            }
            EndTick(game);
            WaitTime(startTime + tickDuration, wait - tick);
        }
    }

    inline void Run(bool& isRunning, Game& game)
    {
        global::LOGIC_THREAD = std::thread(&GameLoop, std::ref(isRunning), std::ref(game));
    }


} // namespace magique::updater