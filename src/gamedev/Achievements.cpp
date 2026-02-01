// SPDX-License-Identifier: zlib-acknowledgement
#include <functional>
#include <cstring>

#include <magique/gamedev/Achievements.h>
#include <magique/assets/JSON.h>
#include <magique/util/Logging.h>

struct Achievement final
{
    std::function<bool()> condition;
    std::string name;
    bool isFinished = false;
};

template <>
struct glz::meta<Achievement>
{
    using T = Achievement;
    static constexpr auto value = object(&T::name, &T::isFinished);
}; // namespace glz

namespace magique
{
    inline std::vector<Achievement> ACHIEVEMENTS{};
    inline AchievementCallback CALL_BACK = nullptr;

    Achievement* GetAchievement(const std::string& name)
    {
        for (auto& a : ACHIEVEMENTS)
        {
            if (a.name == name)
            {
                return &a;
            }
        }
        return nullptr;
    }

    bool AddAchievement(const std::string& name, const std::function<bool()>& constraint)
    {
        const auto* existing = GetAchievement(name);
        if (existing != nullptr)
            return false;

        ACHIEVEMENTS.emplace_back(constraint, name, false);
        return true;
    }

    bool TriggerAchievement(const std::string& name)
    {
        auto* a = GetAchievement(name);
        if (a == nullptr)
            return false;

        a->isFinished = true;

        if (CALL_BACK != nullptr)
            CALL_BACK(a->name);

        return true;
    }

    void SetAchievementCallback(const AchievementCallback callback) { CALL_BACK = callback; }

    void CheckAchievements()
    {
        for (auto& a : ACHIEVEMENTS)
        {
            if (!a.isFinished && a.condition())
            {
                a.isFinished = true;
                if (CALL_BACK != nullptr)
                    CALL_BACK(a.name);
            }
        }
    }

    std::string ExportAchievementsState()
    {
        if (ACHIEVEMENTS.empty())
        {
            LOG_WARNING("Failed to export achievements: No achievements exist!");
            return {nullptr, 0};
        }

        std::string buffer;
        if (!ExportJSON(ACHIEVEMENTS, buffer))
        {
            LOG_WARNING("Failed to export achievements");
        }
        return buffer;
    }

    bool ImportAchievementsState(std::string_view data)
    {
        if (data.empty())
        {
            LOG_WARNING("Failed to import achievements: Invalid data");
            return false;
        }

        std::vector<Achievement> imports;
        if (!ImportJSON(data, imports))
        {
            LOG_WARNING("Failed to import achievements");
            return false;
        }

        int loaded = 0;
        for (auto& import : imports)
        {
            auto local = std::ranges::find_if(ACHIEVEMENTS, [&](auto& a) { return a.name == import.name; });
            if (local == ACHIEVEMENTS.end())
            {
                LOG_WARNING("Achievement %s does not exists locally", import.name.c_str());
            }
            else
            {
                local->isFinished = import.isFinished;
                loaded++;
            }
        }

        LOG_INFO("Imported state of %d achievements", loaded);
        return true;
    }

} // namespace magique
