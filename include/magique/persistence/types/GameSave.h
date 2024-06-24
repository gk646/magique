#ifndef MAGIQUE_GAMESAVE_H
#define MAGIQUE_GAMESAVE_H

#include <string>
#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Game Save
//-----------------------------------------------
// .....................................................................
// POD means Plain Old Data which means that all data is stored directly in the struct.
// For example if you class contains a pointer to something (e.g a vector) this data is stored outside your class
// Note: All save calls copy the data on call. The total data is only persisted when you call SaveGameSave()!
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
        bool saveString(StorageID id, const std::string& string);

        // Saves arbitrary data to the specified slot
        bool saveData(StorageID id, const void* data, int bytes);

        // Saves 1 or many objects of a POD (Plain old data) type
        // To save an array pass the first element and set count to array length
        template <typename T>
        bool saveType(const StorageID id, const T& obj, const int count = 1)
        {
            return saveDataImpl(id, &obj, count, sizeof(T));
        }

        bool saveTable(const DataTable& table);

        //----------------- GETTING -----------------//

        // Returns the string value from the slot
        std::string getString(StorageID id);

        // Returns the data from this slot
        // Optional: Specify the type to get the correct type back
        template <typename T = void>
        T* getData(StorageID id)
        {
            return static_cast<T*>(getDataImpl(id));
        }

        // Returns the data from this slot
        template <typename T>
        T* getType(StorageID id)
        {
            return static_cast<T*>(getDataImpl(id));
        }

    private:
        [[nodiscard]] GameSaveStorageCell* getDataImpl(StorageID id);
        bool saveDataImpl(StorageID id, const char* data, int count, int typeSize);

        bool isPersisted = false;
        std::vector<GameSaveStorageCell> storage; // Internal data holder
    };

} // namespace magique


#endif //MAGIQUE_GAMESAVE_H