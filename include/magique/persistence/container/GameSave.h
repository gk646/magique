#ifndef MAGIQUE_GAMESAVE_H
#define MAGIQUE_GAMESAVE_H

#include <string>
#include <vector>
#include <magique/persistence/types/DataTable.h>

//-----------------------------------------------
// Game Save
//-----------------------------------------------
// .....................................................................
// This is a manual interface to manage game saves (compared to the automatic game config and assets)
// It works by saving plain bytes from the given data which has some limitations for cross-platform saves
// Check the wiki for more infos: https://github.com/gk646/magique/wiki/Persistence
// POD means Plain Old Data which means that all data is stored directly in the struct.
// For example if your class contains a pointer to something (e.g a vector) this data is stored outside your class!
// If you save non-POD types you have to manually handles the data that is stored outside the struct!
// Note: All save calls copy the passed data on call. The total data is only persisted when you call SaveGameSave()!
// Note: All save calls overwrite the existing data of the slot. They are NOT additive.
// .....................................................................

enum class StorageID : int; // User implemented to identify stored information

namespace magique
{
    struct GameSave final
    {
        //----------------- PERSISTENCE -----------------//

        // Persists the given save to disk
        // Failure: Returns false
        static bool Save(GameSave& save, const char* filePath, uint64_t encryptionKey = 0);

        // Loads a save from disk or create a new game save
        // Failure: GameSave.data will be null
        static GameSave Load(const char* filePath, uint64_t encryptionKey = 0);

        //----------------- SAVING -----------------//
        // Note: All data is copied on call

        // Saves a string value to the specified slot
        void saveString(StorageID id, const std::string& string);

        // Saves arbitrary data to the specified slot
        void saveData(StorageID id, const void* data, int bytes);

        // Saves the vector to the specified slot
        // Note: the value-type of the vector should be a POD type (see module header)
        template <typename T>
        void saveVector(StorageID id, const std::vector<T>& vector);

        // Saves the given data table to the specified slot
        template <typename... Types>
        void saveDataTable(StorageID id, const DataTable<Types...>& table);

        //----------------- GETTING -----------------//

        // If the storage exists AND stores a string returns a copy of it
        // Failure: else returns the given default value
        std::string getStringOrElse(StorageID id, const std::string& defaultVal = "");

        // Returns a copy of the data from this slot
        // Optional: Specify the type to get the correct type back
        // Failure: returns {nullptr,0} if the storage doesn't exist or type doesn't match
        template <typename T = void>
        DataPointer<T> getData(StorageID id);

        // Returns a copy of the vector stored at this slot
        // Failure: returns an empty vector
        template <typename T>
        std::vector<T> getVector(StorageID id);

        // Built-In types

        // Returns a fully built data table - includes column names
        template <typename... Types>
        DataTable<Types...> getDataTable(StorageID id);

        //----------------- UTIL -----------------//

        // Returns the storage type of the specified id
        // Failure: if the storage doesnt exist or is empty returns StorageType::EMPTY
        StorageType getStorageInfo(StorageID id);

        GameSave() = default;
        GameSave(const GameSave& other) = delete;            // Involves potentially copying a lot of data
        GameSave& operator=(const GameSave& other) = delete; // Involves potentially copying a lot of data
        GameSave(GameSave&& other) noexcept;
        GameSave& operator=(GameSave&& other) noexcept;
        ~GameSave(); // Will clean itself up automatically

    private:
        [[nodiscard]] internal::GameSaveStorageCell* getCell(StorageID id);
        void assignDataImpl(StorageID id, const void* data, int bytes, StorageType type);
        std::vector<internal::GameSaveStorageCell> storage; // Internal data holder
        bool isPersisted = false;                           // If the game save has been saved to disk
    };

} // namespace magique


//----------------- IMPLEMENTATION -----------------//
namespace magique
{
    template <typename T>
    void GameSave::saveVector(const StorageID id, const std::vector<T>& vector)
    {
        assignDataImpl(id, vector.data(), static_cast<int>(vector.size() * sizeof(T)), StorageType::VECTOR);
    }
    template <typename T>
    DataPointer<T> GameSave::getData(const StorageID id)
    {
        const auto* const cell = getCell(id);
        if (cell == nullptr) [[unlikely]]
        {
            LOG_WARNING("Storage with given id does not exist!");
            return {nullptr, 0};
        }
        if (cell->type != StorageType::DATA)
        {
            LOG_ERROR("This storage does not save data! %d", id);
            return {nullptr, 0};
        }

        const auto size = cell->size;
        if constexpr (std::is_same_v<T, void>)
        {
            auto* copy = new unsigned char[size];
            std::memcpy(copy, cell->data, size);
            return {reinterpret_cast<T*>(copy), size};
        }
        else
        {
            if (size % sizeof(T) != 0)
            {
                LOG_ERROR("Type error - Saved data doesnt match to the given type - You likely used the wrong type or "
                          "wrong storage id!");
                return {nullptr, 0};
            }

            const auto numElements = size / sizeof(T);
            auto* data = new T[numElements];
            std::memcpy(data, cell->data, size);

            return {data, size};
        }
    }
    template <typename T>
    std::vector<T> GameSave::getVector(const StorageID id)
    {
        const auto* const cell = getCell(id);
        if (cell == nullptr) [[unlikely]]
        {
            LOG_ERROR("Storage with given id does not exist!");
            return {};
        }
        if (cell->type != StorageType::VECTOR)
        {
            LOG_ERROR("This storage does not save a vector! %d", id);
            return {};
        }
        std::vector<T> ret(cell->size / sizeof(T));
        std::memcpy(ret.data(), cell->data, cell->size);
        return ret;
    }

} // namespace magique

#endif //MAGIQUE_GAMESAVE_H