// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/persist/GameStorage.h>
#include <magique/util/Logging.h>

namespace magique
{
    bool GameStorageToFile(GameStorage& save, std::string_view path, const uint64_t key)
    {
        return internal::StorageContainer::ToFile(save, path, "GameSave", key);
    }

    bool GameStorageFromFile(GameStorage& save, std::string_view filePath, const uint64_t key)
    {
        return internal::StorageContainer::FromFile(save, filePath, "GameSave", key);
    }

    void GameStorage::saveString(std::string_view slot, const std::string_view& string)
    {
        auto& cell = getCellOrNew(slot, StorageType::STRING);
        cell.data = string;
    }

    void GameStorage::saveBytes(std::string_view slot, const void* data, int bytes)
    {
        assignDataImpl(slot, data, bytes, StorageType::DATA);
    }

    std::string_view GameStorage::getStringOrElse(std::string_view slot, std::string_view defaultVal)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING(defaultVal);
        M_GAMESAVE_TYPE_MISMATCH(STRING, defaultVal);
        return cell->data;
    }

    std::string_view GameStorage::getJSON(std::string_view slot)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING({});
        M_GAMESAVE_TYPE_MISMATCH(JSON, {});
        return cell->data;
    }

    void GameStorage::clear() { clearImpl(); }
    void GameStorage::erase(std::string_view slot) { eraseImpl(slot); }
    StorageType GameStorage::getSlotType(std::string_view slot) { return getSlotTypeImpl(slot); }


} // namespace magique
