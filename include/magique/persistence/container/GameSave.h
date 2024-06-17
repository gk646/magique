#ifndef MAGIQUE_GAMESAVE_H
#define MAGIQUE_GAMESAVE_H

#include <string>
#include <vector>
#include <magique/core/Types.h>
#include <magique/util/Macros.h>

//-----------------------------------------------
// Game Save
//-----------------------------------------------
// .....................................................................
// POD means Plain Old Data which means that all data is stored directly in the struct (in this case).
// For example if you class contains a pointer to something (e.g a vector) this data is stored outside your class
// you have to use the *Serialized versions!
//
// Note: If you change the layout of saved classes you have to save again before they can be loaded!
// Note: Save calls snapshot and copy the data. The save is only saved to file with SaveGameSave();
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

        bool saveString(StorageID id, const std::string& string);

        // Saves a single POD (Plain old data) type
        template <typename T>
        bool saveValue(StorageID id, const T& value)
        {
            return saveData(id, &value, 1, sizeof(T));
        }

        // Saves an array of the given type
        // IMPORTANT: Only works for POD types - no pointers or references ( includes std::vector ...)
        template <typename T>
        bool saveArray(StorageID id, const T* array, const int count)
        {
            M_ASSERT(count >= 0, "Invalid size");
            M_ASSERT(array != nullptr, "Passing nulltpr");
            if (count == 0)
                return true;
            return saveData(id, array, sizeof(T), count);
        }

        // Allows you manually specify how to persists a type
        // IMPORTANT: You can only load this information back with getSerialized()!
        //            Also the deserialization order has to match the serialization order exactly!
        template <typename T, typename SerializeFunc = void (*)(const T&, Serializer&)>
        bool saveSeralized(StorageID id, const T& value, SerializeFunc func)
        {
            Serializer serializer{};
            func(value, serializer);
            auto res = saveData(id, serializer.data, 1, serializer.size);
            delete serializer.data;
            return res;
        }

        //----------------- GETTING -----------------//

        // Returns the string value at this id
        std::string getString(StorageID id);

        // Returns the value saved at this it
        // Note:    You have to specify the type for the C++ type system - returned information will always be the same
        template <typename T>
        T getValue(StorageID id)
        {
            const auto cell = getData(id);
            M_ASSERT(cell != nullptr, "No saved information for this id");
            M_ASSERT(cell->size > 0, "No saved information for this id");
            M_ASSERT(sizeof(T) == cell->size, "Size does not match");
            return *reinterpret_cast<T*>(cell->data);
        }

        // Returns the array data and its count
        // Note:    You have to specify the type for the C++ type system - returned information will always be the same
        // Failure: Returns {nullptr, 0};
        template <typename T>
        std::tuple<T*, int> getArray(StorageID id)
        {
            const auto cell = getData(id);
            M_ASSERT(cell != nullptr, "No saved information for this id");
            return {reinterpret_cast<T*>(cell->data + 4), static_cast<int>(*cell->data)};
        }

        template <typename T, typename DeSerializeFunc = void (*)(const T&, Serializer&)>
        void getSerialized(StorageID id, T& value, DeSerializeFunc func)
        {
            const auto cell = getData(id);
            M_ASSERT(cell != nullptr, "No saved information for this id");
            M_ASSERT(cell->size > 0, "No saved information for this id");
            Serializer serializer{cell->data, cell->size};
            func(value, serializer);
        }

    private:
        [[nodiscard]] StorageCell* getData(StorageID id);
        bool saveData(StorageID id, const char* data, int count, int typeSize);

        bool isPersisted = false;
        std::vector<StorageCell> storage; // Internal data holder
    };

} // namespace magique


#endif //MAGIQUE_GAMESAVE_H