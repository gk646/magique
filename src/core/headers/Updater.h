namespace magique::updater
{
    inline void InternalUpdate(entt::registry& registry)
    {
        global::UI_DATA.update();
        global::AUDIO_PLAYER.update();
        global::COMMAND_LINE.update();
        global::PARTICLE_DATA.update();
        InputSystem(registry);
        LogicSystem(registry);
        CollisionSystem(registry);
    }

    inline void StartTick() { PollInputEvents(); }

    inline double EndTick(const double startTime, Game& game)
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
        const double tickTime = glfwGetTime() - startTime;
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>(tickTime * 1'000'000'000.0F));
        return tickTime;
    }

    inline double Tick(const double startTime, entt::registry& reg, Game& game)
    {
        StartTick();
        //Tick game
        {
            InternalUpdate(reg); // Internal update upfront
            game.updateGame(reg);
        }
        return EndTick(startTime, game);
    }

} // namespace magique::updater