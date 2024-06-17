#ifndef MAGIQUE_GAMESAVE_H
#define MAGIQUE_GAMESAVE_H

#include <vector>
#include <magique/core/Types.h>
#include <magique/util/Macros.h>

//-----------------------------------------------
//
//-----------------------------------------------
// .....................................................................
// .....................................................................

enum class StorageID : int; // User implemented to identify stored information

namespace magique
{
    struct SaveGame final
    {
        SaveGame() = default;
        SaveGame(const char* data, int size); // Internal constructor

        //----------------- SAVING -----------------//

        // Saves a array of the given type
        // Note: supported types: float, int, bool
        template <typename T>
        bool saveArray(StorageID id, const T* array, int size)
        {
            M_ASSERT(size >= 0, "Invalid size");
            M_ASSERT(array != nullptr, "Passing nulltpr");
            return saveData(id, static_cast<void*>(array), size);
        }


        //----------------- GETTING -----------------//

        // Returns the array data and its size
        // Note:    You have to specify the type for the C++ type system - returned information will always be the same
        // Failure: Returns {nullptr, 0};
        template <typename Type>
        std::tuple<Type*, int> getArray(const StorageID id) const
        {
            const auto cell = getData(id);
            if (cell == nullptr)
                return {nullptr, 0};
            return {reinterpret_cast<Type*>(cell->data + 4), static_cast<int>(*cell->data)};
        }


    private:
        const StorageCell* getData(StorageID id) const;
        bool saveData(StorageID id, void* data, int size);

        std::vector<StorageCell> storage; // Internal data holder
    };

} // namespace magique


#endif //MAGIQUE_GAMESAVE_H