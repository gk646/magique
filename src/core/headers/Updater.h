namespace magique::updater
{
    inline void StartTick() { PollInputEvents(); }

    inline double EndTick(const double startTime)
    {
        const double tickTime = GetTime() - startTime;
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>(tickTime * 1'000'000'000.0F));
        return tickTime;
    }

    inline double Tick(const double startTime, entt::registry& reg, Game& game)
    {
        StartTick();
        {
            InternalUpdatePre(reg, game); // Internal update upfront
            game.updateGame(GetGameState());
            InternalUpdatePost();
            game.postTickUpdate(GetGameState());
        }
        return EndTick(startTime);
    }

} // namespace magique::updater