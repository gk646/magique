#ifndef MAGIQUE_GAMECONFIG_H
#define MAGIQUE_GAMECONFIG_H

#include <vector>
#include <string>
#include <magique/internal/InternalTypes.h>

//-----------------------------------------------
// GameConfig
//-----------------------------------------------
// .....................................................................
// The GameConfig is a container for information thats consistent across saves like:
//   - Settings (Video, Gameplay, Accessability...)
//   - Keybinds (if they are not save specific)
//   - Global User Statistics or System
//   - ...
// magique automatically loads (or creates) a config based on the given path in Game::run()
// This global instance can be accessed freely with GetGameConfig() - You should only need this single config per game
// Note: The game config owns and stores the data and is persisted automatically when the game is shutdown gracefully
// .....................................................................

enum class ConfigID; // User implemented - used as direct index

namespace magique
{
    struct GameConfig
    {
        //----------------- PERSISTENCE -----------------//

        // Calls the given function once to initialize the config IF AND ONLY IF the config is empty
        // This is very useful to make sure the config contains some base parameters if the game is opened for the first time
        void initializeIfEmpty(const std::function<void(GameConfig& config)>& func);

        //----------------- SAVE -----------------//

        // Saves a keybind at the given id
        // Example: SaveKeyBind(Keybind(KEY_M), ConfigID::OPEN_MAP);
        void saveKeybind(ConfigID id, Keybind keybind);

        // Saves a string
        // Note: String is passed by value and moved
        void saveString(ConfigID id, std::string string);

        // Saves any primitive datatype (bool, char, int, float, double, ...) plus magique::Point
        template <typename T>
        void saveValue(ConfigID id, T val);

        //----------------- GET -----------------//

        // Returns a modifiable reference to this keybind at the given id
        [[nodiscard]] Keybind& getKeybind(ConfigID id);

        // Returns a modifiable reference to the string at the given id
        [[nodiscard]] std::string& getString(ConfigID id);

        // Returns a modifiable reference to the value at the given id
        // The correct type has to be specified
        template <typename T>
        [[nodiscard]] T& getValue(ConfigID id);

        //----------------- REMOVE -----------------//

        void remove(ConfigID id);

        void clear();

    private:
        static GameConfig LoadFromFile(const char* fName, uint64_t key = 0);
        static void SaveToFile(const GameConfig& config, const char* fName, uint64_t key = 0);
        std::vector<GameConfigStorageCell> storage; // Saves all types except string
        std::vector<std::string> stringStorage;     // Stores strings
        friend struct Game;
    };

} // namespace magique

#endif //MAGIQUE_GAMECONFIG_H