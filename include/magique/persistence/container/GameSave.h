#ifndef MAGIQUE_GAMESAVE_H
#define MAGIQUE_GAMESAVE_H

#include <string>
#include <vector>
#include <magique/core/Types.h>
#include <magique/internal/InternalTypes.h>
#include <magique/internal/Macros.h>

//-----------------------------------------------
// Game Save
//-----------------------------------------------
// .....................................................................
// POD means Plain Old Data which means that all data is stored directly in the struct.
// For example if you class contains a pointer to something (e.g a vector) this data is stored outside your class
// Note: All save calls copy the passed data on call. The total data is only persisted when you call SaveGameSave()!
// Note: All calls overwrite the existing data of the slot. They are NOT additive.
// .....................................................................

enum class StorageID : int; // User implemented to identify stored information

namespace magique
{
    struct GameSave final
    {
        ~GameSave(); // Will clean itself up automatically

        //----------------- LIFE CYCLE -----------------//

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

        // Saves 1 or many objects of a POD (Plain old data) type
        // To save an array pass the first element and set count to array length
        template <typename T>
        void saveType(StorageID id, const T& obj, int count = 1);

        // Savest the given data table to the specified slot
        template <typename... Types>
        void saveTable(StorageID id, const DataTable<Types...>& table);

        //----------------- GETTING -----------------//
        // IMPORTANT: the data is COPIED on each call to a "get" Function!

        // Returns the string value from the slot
        std::string getString(StorageID id);

        // Returns the data from this slot
        // Optional: Specify the type to get the correct type back
        template <typename T = void>
        T* getData(StorageID id);

        // Returns the data from this slot
        template <typename T>
        T* getType(StorageID id);

        // Returns a fully built data table
        // Includes column names
        template <typename... Types>
        DataTable<Types...> getTable(StorageID id)
        {
            return DataTable<Types...>({"hey"});
        }

    private:
        [[nodiscard]] GameSaveStorageCell* getDataImpl(StorageID id);
        void assignDataImpl(StorageID id, const char* data, int bytes);
        void appendDataImpl(StorageID id, const char* data, int bytes);

        bool isPersisted = false;
        std::vector<GameSaveStorageCell> storage; // Internal data holder
    };

} // namespace magique


    //----------------- IMPLEMENTATION -----------------//

template <typename T>
    void magique::GameSave::saveType(const StorageID id, const T& obj, const int count)
    {
        return saveDataImpl(id, &obj, count, sizeof(T));
    }
    template <typename... Types>
    void magique::GameSave::saveTable(const StorageID id, const DataTable<Types...>& table)
    {
        auto data = table.getData();
        return saveDataImpl(id, data.data(), data.size());
    }
    template <typename T>
    T* magique::GameSave::getData(StorageID id)
    {
        M_ASSERT(getDataImpl(id) != nullptr, "Storage with given id does not exist!");
        return static_cast<T*>(getDataImpl(id));
    }
    template <typename T>
    T* magique::GameSave::getType(StorageID id)
    {
        M_ASSERT(getDataImpl(id) != nullptr, "Storage with given id does not exist!");
        return static_cast<T*>(getDataImpl(id));
    }

#endif //MAGIQUE_GAMESAVE_H