#include <algorithm>

#include <magique/ecs/GameSystem.h>
#include <magique/util/Datastructures.h>
#include <magique/core/Camera.h>
#include <magique/core/Engine.h>

#include "internal/Shared.h"

namespace magique
{
    struct GameSystemData final
    {
        bool benchmark = false;

        enum class Function
        {
            DrawGame,
            UpdateGame,
            UpdateEnd,
            COUNT,
        };

        struct SystemEntry
        {
            GameSystem* system = nullptr;
            EnumArray<Function, BenchmarkEntry> functions;

            float getTotalMillis()
            {
                float total = 0;
                for (auto& function : functions)
                {
                    total += function.value.getAvgMillis();
                }
                return total;
            }
        };
        std::vector<SystemEntry> systems;
    };

    inline GameSystemData SYSTEM_DATA{};

#define CALL_FUNCTIONS(function, call)                                                                                  \
    const auto state = EngineGetState();                                                                                \
    for (auto& slot : SYSTEM_DATA.systems)                                                                              \
    {                                                                                                                   \
        if (slot.system->validStates[(int)state] && slot.system->shouldBeCalled())                                      \
        {                                                                                                               \
            double begin = 0;                                                                                           \
            if (SYSTEM_DATA.benchmark)                                                                                  \
            {                                                                                                           \
                begin = GetTime();                                                                                      \
            }                                                                                                           \
            slot.system->on##function call;                                                                             \
            if (SYSTEM_DATA.benchmark)                                                                                  \
            {                                                                                                           \
                slot.functions[GameSystemData::Function::function].add(GetTime() - begin);                              \
            }                                                                                                           \
        }                                                                                                               \
    }

    void GameSystem::CallDrawGame() { CALL_FUNCTIONS(DrawGame, (state, CameraGet())); }

    void GameSystem::CallUpdateGame() { CALL_FUNCTIONS(UpdateGame, (state)); }

    void GameSystem::CallUpdateEnd() { CALL_FUNCTIONS(UpdateEnd, (state)); }

    const char* GameSystem::getName() const { return name.c_str(); }

    void GameSystemRegister(GameSystem* system, const std::string& name, std::initializer_list<GameState> stats)
    {
        if (stats.size() == 0)
        {
            std::memset(system->validStates.data(), 1, sizeof(bool) * system->validStates.size());
        }
        else
        {
            std::ranges::for_each(stats, [&](GameState gs) { system->validStates[(int)gs] = true; });
        }

        system->name = name;
        system->init();
        SYSTEM_DATA.systems.emplace_back(system);
    }

    void GameSystemPrintStats()
    {
        if (!SYSTEM_DATA.benchmark)
        {
            LOG_WARNING("Benchmark for gamesystems not enabled!");
            return;
        }

        auto copy = SYSTEM_DATA.systems;
        std::ranges::sort(copy, [](auto& one, auto& two) { return one.getTotalMillis() > two.getTotalMillis(); });

        LOG_INFO("Gamesystem Stats:");
        printf("\t%-25s || %10s | %10s | %10s \n", "System // Function (ms)", "DrawGame", "UpdateGame", "UpdateEnd");
        for (auto& system : copy)
        {
            auto& func = system.functions;
            printf("\t%-25s || %10.2f | %10.2f | %10.2f \n", system.system->getName(),
                   func[GameSystemData::Function::DrawGame].getAvgMillis(),
                   func[GameSystemData::Function::UpdateGame].getAvgMillis(),
                   func[GameSystemData::Function::UpdateEnd].getAvgMillis());
        }
    }

    void GameSystemEnableStatistics(bool value)
    {
        SYSTEM_DATA.benchmark = value;
        if (value)
        {
            for (auto& system : SYSTEM_DATA.systems)
            {
                for (auto& [key, val] : system.functions)
                {
                    val.reset();
                }
            }
        }
    }

} // namespace magique
