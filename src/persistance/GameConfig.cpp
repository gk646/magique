#include <magique/persistence/types/GameConfig.h>
#include <magique/core/Types.h>
#include <magique/util/Macros.h>

namespace magique
{
    void GameConfig::SaveKeybind(ConfigID id, Keybind keybind)
    {
        if (storage.size() < (int)id + 1)
        {
            storage.resize((int)id + 1);
        }
        storage[(int)id].keybind = keybind;
    }

    void GameConfig::SaveSetting(ConfigID id, Setting setting)
    {
        if (storage.size() < (int)id + 1)
        {
            storage.resize((int)id + 1);
        }
        storage[(int)id].setting = setting;
    }

    void GameConfig::SaveString(ConfigID id, const std::string& string)
    {
        if (storage.size() < (int)id + 1)
        {
            storage.resize((int)id + 1);
        }
        if (storage[(int)id].string != nullptr)
        {
            *static_cast<std::string*>(storage[(int)id].string) = string;
        }
        else
        {
            storage[(int)id].string = new std::string(string);
        }
    }


    Keybind& GameConfig::GetKeybind(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        return storage[(int)id].keybind;
    }

    Setting& GameConfig::GetSetting(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        return storage[(int)id].setting;
    }

    std::string& GameConfig::GetString(ConfigID id)
    {
        M_ASSERT(storage.size() > (int)id, "Setting was never assigned!");
        M_ASSERT(storage[(int)id].string != nullptr, "No string saved to this slot");
        return *static_cast<std::string*>(storage[(int)id].string);
    }


} // namespace magique