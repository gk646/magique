// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_GAME_CONFIG_H
#define MAGIQUE_GAME_CONFIG_H

#include <vector>
#include <string>
#include <magique/internal/InternalTypes.h>

//===============================================
// GameConfig
//===============================================
// .....................................................................
// The GameConfig is a container for information that's consistent across saves like:
//   - Settings (Video, Gameplay, Accessibility...)
//   - Keybinds (if they are not game-save specific)
//   - Global User Statistics or System
//   - ...
// magique automatically loads (or creates) a config based on the given path in Game::run()
// This global instance can be accessed freely with GetGameConfig() - You should only need this single config per game
// Note: The game config is persisted automatically when the game is shutdown gracefully
// .....................................................................

enum class ConfigID; // User implemented - used as direct index

namespace magique
{

    // Returns the global GameConfig instance
    // Note: Only valid to call inside and after Game::onStartup() - will crash earlier
    GameConfig& GetGameConfig();

    struct GameConfig
    {
        //================= SAVE =================//

        // Saves a keybind at the given id
        // Example: saveKeyBind(ConfigID::OPEN_MAP, Keybind{KEY_M});
        void saveKeybind(ConfigID id, Keybind keybind);

        // Saves a string
        // Note: String is passed by value and moved
        void saveString(ConfigID id, const std::string& string);

        // Saves any plain old data type up to 8 bytes
        template <typename T>
        void saveValue(ConfigID id, T val);

        //================= GET =================//

        // If present returns a copy to the keybind at the given id
        // Failure: if the value is not present returns the default
        Keybind getKeybindOrElse(ConfigID id, Keybind defaultKeybind = Keybind{0});

        // If present returns a copy to the string at the given id
        // Failure: if the value is not present returns the default
        [[nodiscard]] std::string getStringOrElse(ConfigID id, const std::string& defaultString = "");

        // If present returns a copy to the value at the given id
        // Failure: if the value is not present returns the default
        template <typename T>
        T getValueOrElse(ConfigID id, const T& defaultValue = {});

        //================= REMOVE =================//

        // Erases the storage with the given id
        void erase(ConfigID id);

        // Erases all storage slots
        void clear();

    private:
        static GameConfig LoadFromFile(const char* filePath, uint64_t key = 0);
        static void SaveToFile(const GameConfig& config, const char* filePath, uint64_t key = 0);
        internal::GameConfigStorageCell* getCell(ConfigID id);
        std::vector<internal::GameConfigStorageCell> storage; // Saves all types except string
        bool loaded = false;
        befriend(Game, GameConfig& GetGameConfig());
    };
} // namespace magique

//================= IMPLEMENTATION =================//

namespace magique
{
    template <typename T>
    void GameConfig::saveValue(const ConfigID id, T val)
    {
        auto* cell = getCell(id);
        if (cell == nullptr)
        {
            internal::GameConfigStorageCell newCell{id};
            newCell.assign(reinterpret_cast<const char*>(&val), sizeof(T), StorageType::VALUE);
            storage.push_back(newCell);
        }
        else
        {
            cell->assign(reinterpret_cast<const char*>(&val), sizeof(T), StorageType::VALUE);
        }
    }
    template <typename T>
    T GameConfig::getValueOrElse(const ConfigID id, const T& defaultValue)
    {
        const auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return defaultValue;
        }
        if (cell->type != StorageType::KEY_BIND)
        {
            return defaultValue;
        }
        T val;
        memcpy(&val, cell->buffer, sizeof(T));
        return val;
    }
} // namespace magique

#endif //MAGIQUE_GAME_CONFIG_H