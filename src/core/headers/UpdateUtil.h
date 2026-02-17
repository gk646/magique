#ifndef UPDATEUTIL_H
#define UPDATEUTIL_H

// Internal updates happen before and after the user update tick

namespace magique
{
    inline void InternalUpdatePre(const entt::registry& registry, Game& game) // Before user space update
    {
        global::UI_DATA.resetConsumed();
        global::TWEEN_DATA.update();
        global::CONSOLE_DATA.update();  // First in case needs to block input
        global::PARTICLE_DATA.update(); // Order doesnt matter
        global::ENGINE_DATA.update();

        LogicSystem(registry); // Before gametick cause essential

        // Order doesnt matter
        auto& config = global::ENGINE_CONFIG;
        if (config.showPerformanceOverlay)
        {
            global::PERF_DATA.updateValues();
        }
        if (config.benchmarkTicks > 0) [[unlikely]]
        {
            config.benchmarkTicks--;
            if (config.benchmarkTicks == 0)
                game.shutDown();
        }

        // Before, so user can react to changes
        static int achieveCounter = 0;
        if (achieveCounter > 30)
        {
            AchievementPoll();
            achieveCounter = 0;
        }
        ++achieveCounter;

        // Before user tick so it gets new information
#ifdef MAGIQUE_STEAM
        global::MP_DATA.update();
        global::STEAM_DATA.update();
#elif MAGIQUE_LAN
        global::MP_DATA.update();
#endif
        global::UI_DATA.onUpdateTick(); // Before user tick so we can layer input
    }

    inline void InternalUpdatePost() // After user space update
    {
        const auto& config = global::ENGINE_CONFIG;
        if (!config.isClientMode && config.enableCollisionSystem) [[likely]]
        {
            // GetMovementDeltas();
            StaticCollisionSystem();  // After cause user systems can modify entity state
            DynamicCollisionSystem(); // After cause user systems can modify entity state
            ResolveCollisions();
        }
        global::AUDIO_PLAYER.update(); // After game tick cause position updates
        WindowManagerGet().update();
    }
} // namespace magique

#endif // UPDATEUTIL_H
