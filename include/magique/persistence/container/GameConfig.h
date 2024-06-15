#ifndef MAGIQUE_GAMECONFIG_H
#define MAGIQUE_GAMECONFIG_H

#include <vector>
#include <string>
#include <magique/core/Types.h>

//-----------------------------------------------
// GameConfig
//-----------------------------------------------
// .....................................................................
// The GameConfig is a save file for information thats consistent across saves like:
//   - Settings (Video, Gameplay, Accessability...)
//   - Keybinds (if they are not save specific)
//   - General Statistics
//   - ...
// .....................................................................

namespace magique
{
    enum class KeybindID : uint16_t;      // User implemented - used as direct index
    enum class SettingID : uint16_t;      // User implemented - used as direct index
    enum class ConfigStringID : uint16_t; // User implemented - used as direct index

    struct GameConfig
    {
        //----------------- SAVE -----------------//

        // Saves a keybind at the given id
        // Example: SaveKeyBind(Keybind(KEY_M), KeybindID::OPEN_MAP);
        void SaveKeybind(Keybind keybind, KeybindID id);

        // Saves a setting at the given id
        void SaveSetting(Setting setting, SettingID id);

        // Saves a string
        void SaveString(const std::string& string, ConfigStringID id);

        //----------------- GET -----------------//

        // Returns a modifiable reference to this keybind
        [[nodiscard]] Keybind& GetKeybind(KeybindID id);

        // Returns a modifiable reference to this setting
        [[nodiscard]] Setting& GetSetting(SettingID id);

        // Returns a modifiable reference to this string
        [[nodiscard]] std::string& GetString(ConfigStringID id);

    private:
        std::vector<Keybind> keybinds;
        std::vector<Setting> settings;
        std::vector<std::string> strings;
    };

} // namespace magique

#endif //MAGIQUE_GAMECONFIG_H