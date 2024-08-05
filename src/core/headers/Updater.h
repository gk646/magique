namespace magique::updater
{
    static double startTime;

    inline void StartTick()
    {
        startTime = glfwGetTime();
        WakeUpJobs();
    }

    inline double EndTick(Game& game)
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
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>(tickTime * 1'000'000'000.0F));
        HibernateJobs(startTime, tickTime);
        return
    }

    inline void Tick(entt::registry& reg, Game& game)
    {
        StartTick();
        //Tick game
        {
            InternalUpdate(reg); // Internal update upfront
            game.updateGame(reg);
        }
        EndTick(game);
    }

} // namespace magique::updater