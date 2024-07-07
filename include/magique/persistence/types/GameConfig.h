#ifndef MAGIQUE_GAMECONFIG_H
#define MAGIQUE_GAMECONFIG_H

#include <vector>
#include <string>
#include <magique/core/Types.h>
#include <magique/internal/Types.h>

//-----------------------------------------------
// GameConfig
//-----------------------------------------------
// .....................................................................
// The GameConfig is a save file for information thats consistent across saves like:
//   - Settings (Video, Gameplay, Accessability...)
//   - Keybinds (if they are not save specific)
//   - General Statistics
//   - ...
// There is only 1 config per game!
// Note: The game config owns and stores the data - at any time you can call GetGameConfig() and access it
// .....................................................................

namespace magique
{
    enum class ConfigID : uint16_t; // User implemented - used as direct index

    struct GameConfig
    {
        //----------------- SAVE -----------------//

        // Saves a keybind at the given id
        // Example: SaveKeyBind(Keybind(KEY_M), KeybindID::OPEN_MAP);
        void SaveKeybind(ConfigID id, Keybind keybind);

        // Saves a setting at the given id
        void SaveSetting(ConfigID id, Setting setting);

        // Saves a string
        void SaveString(ConfigID id, const std::string& string);

        //----------------- GET -----------------//

        // Returns a modifiable reference to this keybind
        [[nodiscard]] Keybind& GetKeybind(ConfigID id);

        // Returns a modifiable reference to this setting
        [[nodiscard]] Setting& GetSetting(ConfigID id);

        // Returns a modifiable reference to this string
        [[nodiscard]] std::string& GetString(ConfigID id);

    private:
        std::vector<GameConfigStorageCell> storage;
    };

} // namespace magique

#endif //MAGIQUE_GAMECONFIG_H