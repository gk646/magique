// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/persist/GameStorage.h>
#include <magique/util/Logging.h>

namespace magique
{
    bool GameStorageToFile(GameStorage& save, std::string_view path, const EncryptionKey key)
    {
        return internal::StorageContainer::ToFile(save, path, key);
    }

    bool GameStorageFromFile(GameStorage& save, std::string_view filePath, const EncryptionKey key)
    {
        return internal::StorageContainer::FromFile(save, filePath, key);
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

    std::optional<std::string_view> GameStorage::getString(std::string_view slot)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING({});
        M_GAMESAVE_TYPE_MISMATCH(STRING, {});
        return cell->data;
    }

    std::optional<std::string_view> GameStorage::getBytes(const std::string_view slot)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING({});
        M_GAMESAVE_TYPE_MISMATCH(DATA, {});
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

    bool GameStorage::erase(std::string_view slot) { return eraseImpl(slot); }

    StorageType GameStorage::getSlotType(std::string_view slot) { return getSlotTypeImpl(slot); }


} // namespace magique
