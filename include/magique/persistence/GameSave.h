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

enum class GameSaveSlot : int; // User implemented to identify different slots

namespace magique
{
    // Persists the given save to disk
    // Failure: Returns false
    bool GameSaveToFile(GameSave& save, const char* filePath, uint64_t encryptionKey = 0);

    // Loads an existing save from disk or creates one at the given path
    // Note: When using steam combine with SteamGetUserDataLocation() to access to correct location
    // Failure: Returns false
    bool GameSaveFromFile(GameSave& save, const char* filePath, uint64_t encryptionKey = 0);

    struct GameSave final
    {
        //================= SAVING =================//

        // Saves a string value to the specified slot
        void saveString(GameSaveSlot id, const std::string& string);

        // Saves arbitrary data to the specified slot
        void saveBytes(GameSaveSlot id, const void* data, int bytes);

        // Saves the vector to the specified slot
        // Note: the value-type of the vector should be a POD type (see module header)
        template <typename T>
        void saveVector(GameSaveSlot id, const std::vector<T>& vector);

        // Serializes the given object to JSON using assets/JSON.h
        template <typename T>
        void saveJSON(GameSaveSlot id, const T& obj);

        //================= GETTING =================//

        // If the storage exists AND stores a string returns a copy of it
        // Failure: else returns the given default value
        std::string getStringOrElse(GameSaveSlot id, const std::string& defaultVal = "");

        // Returns a copy of the data from this slot
        // Optional: Specify the type to get the correct type back
        // Failure: returns {nullptr,0} if the storage doesn't exist or type doesn't match
        template <typename T = unsigned char>
        DataPointer<T> getBytes(GameSaveSlot id);

        // Returns a copy of the vector stored at this slot
        // Failure: returns an empty vector
        template <typename T>
        std::vector<T> getVector(GameSaveSlot id);

        // Parses the data from the JSON into the given object using assets/JSON.h
        template <typename T>
        void getJSON(GameSaveSlot id, T& obj);

        //================= UTIL =================//

        // Returns the storage type of the specified id
        // Failure: if the storage doesn't exist or is empty returns StorageType::EMPTY
        StorageType getStorageInfo(GameSaveSlot id);

        GameSave() = default;
        GameSave(const GameSave& other) = delete;            // Involves potentially copying a lot of data
        GameSave& operator=(const GameSave& other) = delete; // Involves potentially copying a lot of data
        GameSave(GameSave&& other) noexcept = default;
        GameSave& operator=(GameSave&& other) noexcept = default;
        ~GameSave(); // Will clean itself up automatically

    private:
        M_MAKE_PUB()
        internal::StorageCell* getCell(GameSaveSlot id);
        internal::StorageCell* getCellOrNew(GameSaveSlot id, StorageType type);
        void assignDataImpl(GameSaveSlot id, const void* data, int bytes, StorageType type);
        std::vector<internal::StorageCell> storage; // Internal data holder
        bool isPersisted = false;                   // If the game save has been saved to disk
    };

} // namespace magique


//================= IMPLEMENTATION =================//
namespace magique
{
    template <typename T>
    void GameSave::saveVector(const GameSaveSlot id, const std::vector<T>& vector)
    {
        assignDataImpl(id, vector.data(), static_cast<int>(vector.size() * sizeof(T)), StorageType::VECTOR);
    }
    template <typename T>
    void GameSave::saveJSON(GameSaveSlot id, const T& obj)
    {
        auto* cell = getCellOrNew(id, StorageType::JSON);
        JSONExport(obj, cell->data);
    }
    template <typename T>
    DataPointer<T> GameSave::getBytes(const GameSaveSlot id)
    {
        const auto* cell = getCell(id);
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
    std::vector<T> GameSave::getVector(const GameSaveSlot id)
    {
        const auto* cell = getCell(id);
        M_GAMESAVE_SLOT_MISSING({});
        M_GAMESAVE_TYPE_MISMATCH(VECTOR, {});
        std::vector<T> ret(cell->data.size() / sizeof(T));
        std::memcpy(ret.data(), cell->data.data(), cell->data.size());
        return ret;
    }
    template <typename T>
    void GameSave::getJSON(GameSaveSlot id, T& obj)
    {
        const auto* cell = getCell(id);
        M_GAMESAVE_SLOT_MISSING();
        M_GAMESAVE_TYPE_MISMATCH(JSON, );
        JSONImport( cell->data, obj);
    }


} // namespace magique

#endif // MAGIQUE_GAMESAVE_DATA_H
