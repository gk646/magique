// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/persistence/GameSave.h>
#include <magique/util/Logging.h>

namespace magique
{
    bool GameSaveToFile(GameSave& save, const char* path, const uint64_t key)
    {
        return internal::StorageContainer::ToFile(save, path, "GameSave", key);
    }

    bool GameSaveFromFile(GameSave& save, const char* filePath, const uint64_t key)
    {
        return internal::StorageContainer::ToFile(save, filePath, "GameSave", key);
    }

    void GameSave::saveString(std::string_view slot, const std::string_view& string)
    {
        auto& cell = getCellOrNew(slot, StorageType::STRING);
        cell.data = string;
    }

    void GameSave::saveBytes(std::string_view slot, const void* data, int bytes)
    {
        assignDataImpl(slot, data, bytes, StorageType::DATA);
    }

    std::string GameSave::getStringOrElse(std::string_view slot, const std::string& defaultVal)
    {
        const auto* cell = getCell(slot);
        M_GAMESAVE_SLOT_MISSING(defaultVal);
        M_GAMESAVE_TYPE_MISMATCH(STRING, defaultVal);
        return cell->data;
    }

    void GameSave::clear() { clearImpl(); }
    void GameSave::erase(std::string_view slot) { eraseImpl(slot); }
    StorageType GameSave::getSlotType(std::string_view slot) { return getSlotTypeImpl(slot); }


} // namespace magique
