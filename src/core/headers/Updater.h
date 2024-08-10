namespace magique::updater
{
    inline void StartTick() { PollInputEvents(); }

    inline double EndTick(const double startTime)
    {
        const double tickTime = GetTime() - startTime;
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>(tickTime * 1'000'000'000.0F));
        return tickTime;
    }

    inline double Tick(const double startTime, const entt::registry& reg, Game& game)
    {
        StartTick();
        {
            InternalUpdate(reg, game); // Internal update upfront
            game.updateGame(GetGameState());
        }
        return EndTick(startTime);
    }

} // namespace magique::updater