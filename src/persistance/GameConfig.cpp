#include <magique/persistence/container/GameConfig.h>
#include <magique/core/Types.h>
#include <magique/util/Macros.h>

namespace magique
{
    void GameConfig::SaveKeybind(Keybind keybind, KeybindID id)
    {
        if (keybinds.size() < (int)id + 1)
        {
            keybinds.resize((int)id + 1);
        }
        keybinds[(int)id] = keybind;
    }

    void GameConfig::SaveSetting(Setting setting, SettingID id)
    {
        if (settings.size() < (int)id + 1)
        {
            settings.resize((int)id + 1);
        }
        settings[(int)id] = setting;
    }

    void GameConfig::SaveString(std::string&& string, ConfigStringID id)
    {
        if (strings.size() < (int)id + 1)
        {
            strings.resize((int)id + 1);
        }
        strings[(int)id] = std::move(string);
    }


    Keybind& GameConfig::GetKeybind(KeybindID id)
    {
        M_ASSERT(keybinds.size() > (int)id, "Setting was never assigned!");
        return keybinds[(int)id];
    }

    Setting& GameConfig::GetSetting(SettingID id)
    {
        M_ASSERT(settings.size() > (int)id, "Setting was never assigned!");
        return settings[(int)id];
    }

    std::string& GameConfig::GetString(ConfigStringID id)
    {
        M_ASSERT(strings.size() > (int)id, "Setting was never assigned!");
        return strings[(int)id];
    }


} // namespace magique