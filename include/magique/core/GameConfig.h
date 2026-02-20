// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_GAME_CONFIG_H
#define MAGIQUE_GAME_CONFIG_H

#include <magique/assets/JSON.h>

//===============================================
// GameConfig
//===============================================
// .....................................................................
// The GameConfig is a container for information that's consistent across saves like:
//   - Settings (Video, Gameplay, Accessibility...)
//   - Keybinds (if they are not game-save specific)
//   - Global User Statistics or System
//   - ...
// magique automatically loads (or creates) a config based on the given path in Game::run() (if enabled)
// This global instance can be accessed freely with GameConfigGet()
// Note: The game config is persisted automatically when the game is shutdown gracefully
// .....................................................................

namespace magique
{
    // Returns the global GameConfig instance
    // Note: Only valid to call inside and after Game::onStartup() - will crash earlier
    GameConfig& GameConfigGet();

    // Allows to enable or disable the game config creation/loading
    // Note: Must be set BEFORE the Game::run() is called (so usually in main.cpp)
    void GameConfigEnable(bool value);

    struct GameConfig final : internal::StorageContainer
    {
        //================= SAVE =================//

        // Saves a keybind at the given id
        // Example: saveKeyBind(ConfigID::OPEN_MAP, Keybind{KEY_M});
        void saveKeybind(std::string_view slot, Keybind keybind);

        // Saves a string
        // Note: String is passed by value and moved
        void saveString(std::string_view slot, const std::string_view& string);

        // Saves any plain old data type up to 8 bytes
        template <typename T>
        void saveValue(std::string_view slot, T val);

        //================= GET =================//

        // If present returns a copy to the keybind at the given id
        // Failure: if the value is not present returns the default
        Keybind getKeybindOrElse(std::string_view slot, Keybind elseVal = Keybind{});

        // If present returns a copy to the string at the given id
        // Failure: if the value is not present returns the default
        [[nodiscard]] std::string getStringOrElse(std::string_view slot, const std::string& elseVal = "");

        // If present returns a copy to the value at the given id
        // Failure: if the value is not present returns the default
        template <typename T>
        T getValueOrElse(std::string_view slot, const T& defaultValue = {});

        //================= UTIL =================//

        void clear();
        void erase(std::string_view slot);
        StorageType getSlotType(std::string_view slot);
    };

} // namespace magique

//================= IMPLEMENTATION =================//

namespace magique
{
    template <typename T>
    void GameConfig::saveValue(const std::string_view slot, T val)
    {
        struct Value
        {
            T val;
        };
        auto& cell = getCellOrNew(slot, StorageType::VALUE);
        Value value{val};
        JSONExport(value, cell.data);
    }

    template <typename T>
    T GameConfig::getValueOrElse(const std::string_view slot, const T& defaultValue)
    {
        const auto* cell = getCell(slot);
        if (cell == nullptr)
        {
            return defaultValue;
        }
        if (cell->type != StorageType::VALUE)
        {
            return defaultValue;
        }
        struct Value
        {
            T val;
        };
        Value value{};
        JSONImport(cell->data, value);
        return value.val;
    }
} // namespace magique

#endif // MAGIQUE_GAME_CONFIG_H
