#include <functional>

#include <magique/gamedev/Achievements.h>
#include <magique/util/Logging.h>
#include <magique/core/Types.h>

#include "internal/datastructures/VectorType.h"

namespace magique
{
    vector<Achievement> ACHIEVEMENTS;
    AchievementCallback CALL_BACK = nullptr;

    Achievement* GetAchievement(const std::string& name)
    {
        for (auto& a : ACHIEVEMENTS)
        {
            if (strcmp(a.name, name.c_str()))
                return &a;
        }
        return nullptr;
    }

    bool AddAchievement(const std::string& name, const std::function<bool()>& constraint)
    {
        const auto existing = GetAchievement(name);
        if (existing != nullptr)
            return false;

        const int len = static_cast<int>(name.size());
        auto* buff = new char[len + 1];
        memcpy(buff, name.c_str(), len);
        buff[len] = '\0'; // fix null terminator

        ACHIEVEMENTS.emplace_back(buff, (void*)new std::function(constraint));
        return true;
    }

    bool TriggerAchievement(const std::string& name)
    {
        auto a = GetAchievement(name);
        if (a == nullptr)
            return false;

        a->finished = true;

        if (CALL_BACK)
            CALL_BACK(*a);

        return true;
    }

    void SetAchievementCallback(const AchievementCallback callback) { CALL_BACK = callback; }

    void CheckAchievements()
    {
        for (auto& a : ACHIEVEMENTS)
        {
            if (!a.finished && (*static_cast<std::function<bool()>*>(a.condition))())
            {
                a.finished = true;
                if (CALL_BACK)
                    CALL_BACK(a);
            }
        }
    }

    std::pair<const unsigned char*, int> GetAchievementsData()
    {
        if (ACHIEVEMENTS.empty())
        {
        }
        return {nullptr, 0};
    }

    bool LoadAchievements(const unsigned char* data, const int size)
    {
        if (data == nullptr || size == 0)
        {
            LOG_ERROR("Invalid data - Failed to load achievements");
            return false;
        }
        LOG_FATAL("Not implemented");
    }

} // namespace magique