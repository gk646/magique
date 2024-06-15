#pragma once

#include <magique/ecs/ECS.h>
#include <magique/util/Defines.h>

#include "ecs/systems/CollisionSystem.h"
#include "ecs/systems/InputSystem.h"
#include "ecs/systems/LogicSystem.h"

namespace magique::updater
{

    using namespace std::chrono;
    inline static time_point<steady_clock> startTime;

    inline void InternalUpdate(entt::registry& registry)
    {
        PollInputs(registry);
        UpdateLogic(registry);
        CheckCollisions(registry);
    }

    inline void StartUpdateTick() {

    }

    inline void EndUpdateTick()
    {
        if (global::CONFIGURATION.showPerformanceOverlay)
        {
            global::PERF_DATA.perfOverlay.updateValues();
        }
        TickInputEvents();
        global::PERF_DATA.saveTickTime(UPDATE, static_cast<uint32_t>((steady_clock::now() - startTime).count()));
    }

    inline void GameLoop(const bool& isRunning, Game& game)
    {
        constexpr auto tickDuration = microseconds(1'000'000 / MAGIQUE_LOGIC_TICKS);

        auto lastTime = steady_clock::now();
        microseconds accumulator(0);
        auto& reg = REGISTRY;

        while (isRunning) [[likely]]
        {
            startTime = steady_clock::now();
            const auto passedTime = duration_cast<microseconds>(startTime - lastTime);
            lastTime = startTime;
            accumulator += passedTime;

            while (accumulator >= tickDuration) [[unlikely]] // Safe guard to close instantly
            {
                StartUpdateTick();
                //Tick game
                {
                    InternalUpdate(reg); // Internal update upfront
                    game.updateGame(reg);
                }
                EndUpdateTick();
                accumulator = microseconds(0);
            }
            std::this_thread::sleep_for(microseconds(1));
        }
    }

    inline void Run(bool& isRunning, Game& game)
    {
        global::LOGIC_THREAD = std::thread(&GameLoop, std::ref(isRunning), std::ref(game));
        LOG_INFO("Started Gameloop");
    }

    inline void Close()
    {
        if (global::LOGIC_THREAD.joinable())
        {
            global::LOGIC_THREAD.join();
        }
    }

} // namespace magique::updater