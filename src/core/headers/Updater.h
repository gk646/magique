namespace magique::updater
{
    inline void StartTick()
    {
        PollInputEvents();
    }

    inline double EndTick(const double startTime, Game& game)
    {
        auto& config = global::ENGINE_CONFIG;
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
        const double tickTime = GetTime() - startTime;
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>(tickTime * 1'000'000'000.0F));
        return tickTime;
    }

    inline double Tick(const double startTime, entt::registry& reg, Game& game)
    {
        StartTick();
        {
            InternalUpdate(reg); // Internal update upfront
            game.updateGame(reg);
        }
        return EndTick(startTime, game);
    }

} // namespace magique::updater