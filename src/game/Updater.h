#pragma once


#include <magique/ecs/Registry.h>
#include <magique/core/Defines.h>

#include "ecs/systems/CollisionSystem.h"

namespace magique::updater
{

    using namespace std::chrono;
    inline static time_point<steady_clock> startTime;

    inline void InternalUpdate(entt::registry& registry) { ecs::CheckCollisions(registry); }

    inline void StartUpdateTick(Game& game) { startTime = steady_clock::now(); }

    inline void EndUpdateTick() { PERF_DATA.saveTickTime(UPDATE, (steady_clock::now() - startTime).count()); }

    inline void GameLoop(const bool& isRunning, Game& game)
    {
        constexpr auto tickDuration = microseconds(1'000'000 / MAGIQUE_LOGIC_TICKS);

        auto lastTime = steady_clock::now();
        microseconds accumulator(0);

        while (isRunning) [[likely]]
        {
            startTime = steady_clock::now();
            const auto passedTime = duration_cast<microseconds>(startTime - lastTime);
            lastTime = startTime;
            accumulator += passedTime;

            while (accumulator >= tickDuration && isRunning) [[unlikely]] // Safe guard to close instantly
            {
                StartUpdateTick(game);
                //Tick game
                {
                    auto& reg = ecs::GetRegistry();
                    InternalUpdate(reg); // Internal update upfront
                    game.updateGame(reg);
                }
                EndUpdateTick();
                accumulator -= tickDuration;
            }
            std::this_thread::sleep_for(microseconds(1));
        }
    }

    inline void Run(bool& isRunning, Game& game)
    {
        LOGIC_THREAD = std::thread(&GameLoop, std::ref(isRunning), std::ref(game));
        LOG_INFO("Started Gameloop");
    }

    inline void Close()
    {
        if (LOGIC_THREAD.joinable())
        {
            LOGIC_THREAD.join();
        }
    }

} // namespace magique::updater