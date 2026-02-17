// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_GAMESAVE_DATA_H
#define MAGIQUE_GAMESAVE_DATA_H

#include <magique/assets/JSON.h>

//===============================================
// GameSaveData
//===============================================
// .....................................................................
// This is a MANUAL interface to manage game save data.
// POD means Plain Old Data which means that all data is stored directly in the struct.
// For example if your class contains a pointer to something (e.g a vector) this data is stored outside your class!
// If you save non-POD types you have to manually handles the data that is stored outside the struct!
// Note: All save calls copy the passed data on call. The total data is only persisted when you call GameSaveToFile()!
// Note: All save calls overwrite the existing data of the slot. They are NOT additive.
// .....................................................................

namespace magique
{
    // Persists the given save to disk
    // Failure: Returns false
    bool GameSaveToFile(GameSave& save, const char* path, uint64_t key = 0);

    // Loads an existing save from disk or creates one at the given path
    // Note: When using steam combine with SteamGetUserDataLocation() to access to correct location
    // Failure: Returns false
    bool GameSaveFromFile(GameSave& save, const char* filePath, uint64_t key = 0);

    struct GameSave final : internal::StorageContainer
    {
        //================= SAVING =================//

        // Saves a string value to the specified slot
        void saveString(std::string_view slot, const std::string_view& string);

        // Saves arbitrary data to the specified slot
        void saveBytes(std::string_view slot, const void* data, int bytes);

        // Saves the vector to the specified slot
        // Note: the value-type of the vector should be a POD type (see module header)
        template <typename T>
        void saveVector(std::string_view slot, const std::vector<T>& vector);

        // Serializes the given object to JSON using assets/JSON.h
        template <typename T>
        void saveJSON(std::string_view slot, const T& obj);

        //================= GETTING =================//

        // If the storage exists AND stores a string returns a copy of it
        // Failure: else returns the given default value
        std::string getStringOrElse(std::string_view slot, const std::string& defaultVal = "");

        // Returns a copy of the data from this slot
        // Optional: Specify the type to get the correct type back
        // Failure: returns {nullptr,0} if the storage doesn't exist or type doesn't match
        template <typename T = unsigned char>
        DataPointer<T> getBytes(std::string_view slot);

        // Returns a copy of the vector stored at this slot
        // Failure: returns an empty vector
        template <typename T>
        std::vector<T> getVector(std::string_view slot);

        // Parses the data from the JSON into the given object using assets/JSON.h
        template <typename T>
        void getJSON(std::string_view slot, T& obj);

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
    void GameSave::saveVector(const std::string_view slot, const std::vector<T>& vector)
    {
        assignDataImpl(slot, vector.data(), static_cast<int>(vector.size() * sizeof(T)), StorageType::VECTOR);
    }

    template <typename T>
    void GameSave::saveJSON(std::string_view slot, const T& obj)
    {
        auto* cell = getCellOrNew(slot, StorageType::JSON);
        JSONExport(obj, cell->data);
    }

    template <typename T>
    DataPointer<T> GameSave::getBytes(const std::string_view slot)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING(DataPointer<T>(nullptr, 0));
        M_GAMESAVE_TYPE_MISMATCH(DATA, DataPointer<T>(nullptr, 0));
        const auto size = (int)cell->data.size();
        if constexpr (std::is_same_v<T, unsigned char>)
        {
            auto* copy = new unsigned char[size];
            std::memcpy(copy, cell->data.data(), size);
            return DataPointer{copy, size};
        }
        else
        {
            if (size % sizeof(T) != 0)
            {
                LOG_ERROR("Type error - Saved data doesnt match to the given type - You likely used the wrong type or "
                          "wrong slot!");
                return {nullptr, 0};
            }
            const auto numElements = size / sizeof(T);
            auto* data = new T[numElements];
            std::memcpy(data, cell->data.data(), size);
            return {data, size};
        }
    }
    template <typename T>
    std::vector<T> GameSave::getVector(const std::string_view slot)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING({});
        M_GAMESAVE_TYPE_MISMATCH(VECTOR, {});
        std::vector<T> ret(cell->data.size() / sizeof(T));
        std::memcpy(ret.data(), cell->data.data(), cell->data.size());
        return ret;
    }
    template <typename T>
    void GameSave::getJSON(std::string_view slot, T& obj)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING();
        M_GAMESAVE_TYPE_MISMATCH(JSON, );
        JSONImport(cell->data, obj);
    }


} // namespace magique

#endif // MAGIQUE_GAMESAVE_DATA_H
