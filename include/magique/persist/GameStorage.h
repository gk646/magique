// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_GAMESAVE_DATA_H
#define MAGIQUE_GAMESAVE_DATA_H

#include <magique/assets/JSON.h>
#include <magique/internal/InternalTypes.h>

//===============================================
// Game Storage
//===============================================
// .....................................................................
// This is a MANUAL interface to store/load data with enforced type checking.
// When saving to a given slot the type of this slot is set according to the method used (e.g. saveString => STRING)
// Retrieval is then only possible when using the correct get method for that type (e.g. getString())
//
// Note: All save calls copy the passed data on call. The total data is only persisted after calling GameStorageToFile()!
// Note: All save calls overwrite the existing data of that slot - They are NOT additive.
// POD means plain old data e.g. your class does not contain pointers or other contains but just flat data members
// .....................................................................

namespace magique
{
    // Persists the given save to disk
    // Failure: Returns false if writing to disk failed
    // Note: file is compressed automatically
    bool GameStorageToFile(GameStorage& save, std::string_view path, EncryptionKey key = 0);

    // Loads an existing save from disk or creates one at the given path
    // Note: When using steam combine with SteamGetUserDataLocation() to access to correct location
    // Failure: Returns false storage could not be loaded
    bool GameStorageFromFile(GameStorage& save, std::string_view filePath, EncryptionKey key = 0);

    struct GameStorage final : internal::StorageContainer
    {
        //================= SAVING =================//

        // Saves a string value to the specified slot
        void saveString(std::string_view slot, std::string_view string);

        // Saves arbitrary data to the specified slot
        void saveBytes(std::string_view slot, const void* data, int bytes);

        // Saves the vector to the specified slot
        // Note: the value-type of the vector should be a POD type (see header info) - else probably use JSON
        template <typename T>
        void saveVector(std::string_view slot, const std::vector<T>& vector);

        // Saves the object serialized to JSON and returns a view to the serialized data
        template <typename T>
        std::string_view saveAsJSON(std::string_view slot, const T& obj);

        //================= GETTING =================//

        // Returns a view to the string stored at the given slot
        std::optional<std::string_view> getString(std::string_view slot);

        // Returns a view to the data at the given slot
        std::optional<std::string_view> getBytes(std::string_view slot);

        // Returns a view to the vector data stored at this slot
        // Failure: returns an empty view
        template <typename T>
        std::span<T> getVector(std::string_view slot);

        // Parses the data from the JSON into the given object
        template <typename T>
        void getFromJSON(std::string_view slot, T& obj);

        // Returns the JSON as view if present
        // Failure: empty view
        std::string_view getJSON(std::string_view slot);

        // Allows iteration over storage cells
        auto begin() const;
        auto end() const;

        //================= UTIL =================//

        // Clears all data
        void clear();

        // Returns true if the given slot was removed
        bool erase(std::string_view slot);

        // Returns the type of the given slot
        StorageType getSlotType(std::string_view slot);
    };

} // namespace magique


//================= IMPLEMENTATION =================//
namespace magique
{
    template <typename T>
    void GameStorage::saveVector(const std::string_view slot, const std::vector<T>& vector)
    {
        assignDataImpl(slot, vector.data(), static_cast<int>(vector.size() * sizeof(T)), StorageType::VECTOR);
    }

    template <typename T>
    std::string_view GameStorage::saveAsJSON(std::string_view slot, const T& obj)
    {
        auto& cell = getCellOrNew(slot, StorageType::JSON);
        JSONExport<false>(obj, cell.data);
        return cell.data;
    }

    template <typename T>
    std::span<T> GameStorage::getVector(const std::string_view slot)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING({});
        M_GAMESAVE_TYPE_MISMATCH(VECTOR, {});
        return std::span{(T*)cell->data.data(), cell->data.size() / sizeof(T)};
    }

    template <typename T>
    void GameStorage::getFromJSON(std::string_view slot, T& obj)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING();
        M_GAMESAVE_TYPE_MISMATCH(JSON, );
        JSONImport(cell->data, obj);
    }

    inline auto GameStorage::begin() const { return cells.begin(); }

    inline auto GameStorage::end() const { return cells.end(); }

} // namespace magique

#endif // MAGIQUE_GAMESAVE_DATA_H
