#ifndef MAGIQUE_GAMESAVE_H
#define MAGIQUE_GAMESAVE_H

#include <string>
#include <vector>
#include <magique/internal/Macros.h>
#include <magique/internal/InternalTypes.h>
#include <magique/persistence/types/DataTable.h>

//-----------------------------------------------
// Game Save
//-----------------------------------------------
// .....................................................................
// This is a manual interface to manage gamesaves (compared to the automatic game config and assets)
// POD means Plain Old Data which means that all data is stored directly in the struct.
// For example if you class contains a pointer to something (e.g a vector) this data is stored outside your class!
// Note: All save calls copy the passed data on call. The total data is only persisted when you call SaveGameSave()!
// Note: All save calls overwrite the existing data of the slot. They are NOT additive.
// .....................................................................

enum class StorageID : int; // User implemented to identify stored information

namespace magique
{
    struct GameSave final
    {
        //----------------- PERSISTENCE -----------------//

        // Loads a save from disk
        // Failure: GameSave.data will be null
        static GameSave LoadGameSave(const char* filePath, uint64_t encryptionKey = 0);

        // Persists the given save to disk
        // Failure: Returns false
        static bool SaveGameSave(GameSave& save, const char* filePath, uint64_t encryptionKey = 0);

        //----------------- SAVING -----------------//

        // Saves a string value to the specified slot
        void saveString(StorageID id, const std::string& string);

        // Saves arbitrary data to the specified slot
        void saveData(StorageID id, const void* data, int bytes);

        // Saves the vector to the specified slot
        // Note: the type of the vector must be a POD type (see module header)
        template <typename T>
        void saveVector(StorageID id, const std::vector<T>& vector);

        // Savest the given data table to the specified slot
        template <typename... Types>
        void saveDataTable(StorageID id, const DataTable<Types...>& table);

        //----------------- GETTING -----------------//

        // If the storage cell exists AND stores a string returns a copy of it
        // Failure: else returns the given default value
        std::string getStringOrElse(StorageID id, const std::string& defaultVal);

        // Returns a pointer to the data from this slot
        // Optional: Specify the type to get the correct type back
        template <typename T = void>
        T* getData(StorageID id);

        template <typename T>
        std::vector<T> getVector(StorageID id);

        // Built-In types

        // Returns a fully built data table - includes column names
        template <typename... Types>
        DataTable<Types...> getTable(StorageID id);

        //----------------- UTIL -----------------//

        // Returns the storage type of the specified cell
        // Failure: if the cell doesnt exist returns StorageType::EMPTY
        StorageType getStorageInfo(StorageID id);

        GameSave() = default;
        GameSave(const GameSave& other);
        GameSave(GameSave&& other) noexcept;
        GameSave& operator=(const GameSave& other) = delete;
        GameSave& operator=(GameSave&& other) noexcept = delete;
        ~GameSave(); // Will clean itself up automatically

    private:
        [[nodiscard]] GameSaveStorageCell* getCell(StorageID id);
        void assignDataImpl(StorageID id, const char* data, int bytes);
        std::vector<GameSaveStorageCell> storage; // Internal data holder
        std::vector<std::string> stringStorage;
        bool isPersisted = false;
    };

} // namespace magique


//----------------- IMPLEMENTATION -----------------//
namespace magique
{
    template <typename T>
    void GameSave::saveType(const StorageID id, const T& obj, const int count)
    {
        return saveDataImpl(id, &obj, count, sizeof(T));
    }
    template <typename... Types>
    void GameSave::saveTable(const StorageID id, const DataTable<Types...>& table)
    {
        auto data = table.getData();
        return saveDataImpl(id, data.data(), data.size());
    }
    template <typename T>
    T* GameSave::getData(const StorageID id)
    {
        M_ASSERT(getDataImpl(id) != nullptr, "Storage with given id does not exist!");
        return static_cast<T*>(getCell(id));
    }
    template <typename T>
    T& GameSave::getType(const StorageID id)
    {
        M_ASSERT(getDataImpl(id) != nullptr, "Storage with given id does not exist!");
        return *static_cast<T*>(getCell(id));
    }
    template <typename... Types>
    DataTable<Types...> GameSave::getTable(StorageID id)
    {
        return DataTable<Types...>({"hey"});
    }

} // namespace magique

#endif //MAGIQUE_GAMESAVE_H