#ifndef UPDATEUTIL_H
#define UPDATEUTIL_H

// Internal updates happen before and after the user update tick

namespace magique
{
    inline void InternalUpdatePre(const entt::registry& registry, Game& game) // Before user space update
    {
        global::CONSOLE_DATA.update();  // First in case needs to block input
        InputSystem();                  // Before gametick per contract (scripting system)
        global::PARTICLE_DATA.update(); // Order doesnt matter
        LogicSystem(registry);          // Before gametick cause essential
                                        //        global::PATH_DATA.updateDynamicGrid();
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
            CheckAchievements();
            achieveCounter = 0;
        }
        ++achieveCounter;

        // Reset nearby query
        global::ENGINE_DATA.nearbyQueryData.lastRadius = 0;
        // Before user tick so it gets new information
#ifdef MAGIQUE_STEAM
        global::MP_DATA.update();
        global::STEAM_DATA.update();
#elif MAGIQUE_LAN
        global::MP_DATA.update();
#endif
    }

    inline void InternalUpdatePost() // After user space update
    {
        const auto& config = global::ENGINE_CONFIG;
        if (!config.isClientMode && config.enableCollisionSystem) [[likely]]
        {
            StaticCollisionSystem();  // After cause user systems can modify entity state
            DynamicCollisionSystem(); // After cause user systems can modify entity state
        }

        global::UI_DATA.update();      // After gametick so ui reflects current state
        global::AUDIO_PLAYER.update(); // After game tick cause position updates
        GetWindowManager().update();
    }
} // namespace magique

#endif //UPDATEUTIL_H