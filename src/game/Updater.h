#pragma once

#include <thread>

#include <magique/ecs/Registry.h>

#include "ecs/systems/CollisionSystem.h"
#include "core/Config.h"

namespace magique::updater
{
    inline std::thread LOGIC_THREAD;

    inline void InternalUpdate(entt::registry& registry)
    {

        ecs::CheckCollisions(registry);
    }

    inline void GameLoop(const bool& isRunning, Game& game)
    {
        using namespace std::chrono;
        using namespace std::this_thread;

        LOG_INFO("Started Gameloop");
        constexpr auto tickDuration = microseconds(1'000'000 / MAGIQUE_LOGIC_TICKS);

        auto lastTime = steady_clock::now();
        microseconds accumulator(0);

        while (isRunning) [[likely]]
        {
            auto now = steady_clock::now();
            const auto passedTime = duration_cast<microseconds>(now - lastTime);
            lastTime = now;
            accumulator += passedTime;

            while (accumulator >= tickDuration && isRunning) [[unlikely]] // Safe guard to close instantly
            {
                const auto startTime = steady_clock::now();
                //Tick game
                {
                    auto& reg = ecs::GetRegistry();
                    InternalUpdate(reg); // Internal update upfront
                    game.updateGame(reg);
                }
                const auto tickTime = steady_clock::now() - startTime;
#ifdef MAGIQUE_DEBUG
                // Glob::GDT.logicTimes.push_back(tickTime.count());
#endif
                accumulator -= tickDuration;
            }

            sleep_for(microseconds(1));
        }
    }

    inline void Run(bool& isRunning, Game& game)
    {
        LOGIC_THREAD = std::thread(&GameLoop, std::ref(isRunning), std::ref(game));
    }

    inline void Close()
    {
        if (LOGIC_THREAD.joinable())
        {
            LOGIC_THREAD.join();
        }
    }

} // namespace magique::updater