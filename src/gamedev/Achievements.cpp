#include <functional>

#include <magique/gamedev/Achievements.h>
#include <magique/util/Logging.h>

#include "internal/datastructures/VectorType.h"

namespace magique
{
    struct Achievement final
    {
        std::function<bool()> condition;
        std::string name;
        bool isFinished = false;
    };

    vector<Achievement> ACHIEVEMENTS;
    AchievementCallback CALL_BACK = nullptr;

    Achievement* GetAchievement(const std::string& name)
    {
        for (auto& a : ACHIEVEMENTS)
        {
            if (strcmp(a.name.c_str(), name.c_str()) == 0)
                return &a;
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

    DataPointer<const unsigned char> GetAchievementsData()
    {
        if (ACHIEVEMENTS.empty())
        {
            LOG_WARNING("No achievements exist!");
            return {nullptr, 0};
        }

        int totalSize = 0;
        for (const auto& a : ACHIEVEMENTS)
        {
            totalSize += a.name.size() + 1; // Name
            ++totalSize;                    // 1 Bytes for completion state
        }

        auto* data = new unsigned char[totalSize];
        int i = 0;
        for (const auto& a : ACHIEVEMENTS)
        {
            const int len = static_cast<int>(a.name.size());
            std::memcpy(&data[i], a.name.data(), len);
            i += len;
            data[i] = '\0';
            ++i;
            data[i] = a.isFinished ? 1 : 0;
            ++i;
        }
        return {data, totalSize};
    }

    bool LoadAchievements(const unsigned char* data, const int size)
    {
        if (data == nullptr || size == 0)
        {
            LOG_WARNING("Invalid data - Failed to load achievements");
            return false;
        }

        std::string cache(20, '\0');
        int saved = 0;    // How many achievements were saved total
        int notFound = 0; // How many saved ones are not found
        int i = 0;
        while (i < size)
        {
            const auto nameLength = (int)strlen(reinterpret_cast<const char*>(&data[i]));
            if (i + nameLength >= size)
            {
                LOG_ERROR("Corrupted data - Achievement name exceeds bounds");
                return false;
            }
            cache.assign(reinterpret_cast<const char*>(&data[i]), nameLength);
            i += nameLength + 1;

            if (i >= size)
            {
                LOG_ERROR("Corrupted data - Achievement state missing");
                return false;
            }

            bool found = false;
            for (auto& a : ACHIEVEMENTS)
            {
                if (strcmp(a.name.c_str(), cache.c_str()) == 0)
                {
                    a.isFinished = data[i] == 1;
                    found = true;
                    break;
                }
            }

            if (!found)
                ++notFound;

            ++i;
            ++saved;
        }

        const auto current = ACHIEVEMENTS.size();
        if (current > saved)
        {
            LOG_WARNING("More achievements present than save: %d / %d", current, saved);
        }
        else if (current < saved)
        {
            LOG_WARNING("More achievements saved than present: %d / %d", saved, current);
        }
        if (notFound > 0)
            LOG_WARNING("Couldnt locate %d saved achievements", notFound);

        LOG_INFO("Successfully loaded achievements: %d", saved);
        return true;
    }

} // namespace magique