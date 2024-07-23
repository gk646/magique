#pragma once

namespace magique::updater
{
    using namespace std::chrono;
    inline static time_point<steady_clock> startTime;

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

    inline void StartTick() { WakeUpJobs(); }

    inline void EndTick()
    {
        if (global::CONFIGURATION.showPerformanceOverlay)
        {
            global::PERF_DATA.perfOverlay.updateValues();
        }
        HibernateJobs();
        TickInputEvents();
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>((steady_clock::now() - startTime).count()));
    }

    inline void GameLoop(const bool& isRunning, Game& game)
    {
        Setup();
        constexpr auto tickDuration = nanoseconds(1'000'000'000 / MAGIQUE_LOGIC_TICKS);

        auto lastTime = steady_clock::now();
        nanoseconds accumulator(0);
        auto& reg = internal::REGISTRY;

        while (isRunning) [[likely]]
        {
            startTime = steady_clock::now();
            const auto passedTime = startTime - lastTime;
            lastTime = startTime;
            accumulator += passedTime;

            while (accumulator >= tickDuration) [[unlikely]] // Safe guard to close instantly
            {
                StartTick();
                //Tick game
                {
                    InternalUpdate(reg); // Internal update upfront
                    game.updateGame(reg);
                }
                EndTick();
                accumulator = nanoseconds::zero();
            }
            std::this_thread::sleep_for(microseconds(1));
        }
    }

    inline void Run(bool& isRunning, Game& game)
    {
        global::LOGIC_THREAD = std::thread(&GameLoop, std::ref(isRunning), std::ref(game));
    }


} // namespace magique::updater