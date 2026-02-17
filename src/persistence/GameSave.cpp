// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS

#include <magique/persistence/GameSave.h>
#include <magique/util/Logging.h>

#include "internal/utils/EncryptionUtil.h"
#include "magique/assets/JSON.h"

namespace magique
{
    constexpr auto* FILE_HEADER = "MAGIQUE_SAVE_FILE";

    StorageType GameSave::getStorageInfo(const GameSaveSlot id)
    {
        const auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return StorageType::EMPTY;
        }
        return cell->type;
    }

    GameSave::~GameSave()
    {
        if (!isPersisted && !storage.empty())
        {
            LOG_WARNING("GameSaveData is deleted without being saved!");
        }
    }

    //----------------- PERSISTENCE -----------------//

    bool GameSaveToFile(GameSave& save, const char* filePath, const uint64_t encryptionKey)
    {
        std::string buffer;
        buffer.reserve(1024);
        JSONExport(save.storage, buffer);

        SymmetricEncrypt(buffer.data(), buffer.size(), encryptionKey);

        FILE* file = fopen(filePath, "wb");
        if (file == nullptr)
        {
            LOG_ERROR("Failed to open file for writing: %s", filePath);
            return false;
        }
        setvbuf(file, nullptr, _IONBF, 0);
        fwrite(FILE_HEADER, strlen(FILE_HEADER), 1, file);
        fwrite(buffer.data(), buffer.size(), 1, file);
        fclose(file);
        save.isPersisted = true;
        LOG_INFO("Saved gamesave: %s | Size: %.2fkb", filePath, (float)buffer.size() / 1000.0F);
        return true;
    }

    bool GameSaveFromFile(GameSave& save, const char* filePath, const uint64_t encryptionKey)
    {
        MAGIQUE_ASSERT(save.isPersisted == false, "Can only load from empty save!");
        MAGIQUE_ASSERT(save.storage.empty(), "Can only load from empty save!");

        save.isPersisted = true; // The loaded gamesave is not expected to be saved (?)

        FILE* file = fopen(filePath, "rb");
        if (file == nullptr)
        {
            LOG_WARNING("File does not exist. Will be created once you save: %s", filePath);
            return false;
        }

        auto constexpr headerSize = static_cast<int>(std::char_traits<char>::length(FILE_HEADER));

        fseek(file, 0, SEEK_END);
        const int totalSize = static_cast<int>(ftell(file)) - headerSize;
        fseek(file, 0, SEEK_SET);

        std::string buffer(totalSize, '\0');
        fread(buffer.data(), totalSize, 1, file);
        fclose(file);

        if (std::memcmp(FILE_HEADER, buffer.data(), headerSize) != 0)
        {
            LOG_ERROR("Malformed gamesave file: %s", filePath);
            return false;
        }

        // Decrypt
        SymmetricEncrypt(buffer.data(), totalSize, encryptionKey);

        JSONImport(buffer, save.storage);

        LOG_INFO("Loaded gamesave: %s | Size: %.2fkb", filePath, static_cast<float>(totalSize) / 1000.0F);
        return true;
    }

    //----------------- SAVING -----------------//

    void GameSave::saveString(const GameSaveSlot id, const std::string& string)
    {
        assignDataImpl(id, string.data(), static_cast<int>(string.size()), StorageType::STRING);
    }

    void GameSave::saveBytes(const GameSaveSlot id, const void* data, const int bytes)
    {
        assignDataImpl(id, data, bytes, StorageType::DATA);
    }

    //----------------- GETTING -----------------//

    std::string GameSave::getStringOrElse(const GameSaveSlot id, const std::string& defaultVal)
    {
        const auto* cell = getCell(id);
        M_GAMESAVE_SLOT_MISSING(defaultVal);
        M_GAMESAVE_TYPE_MISMATCH(STRING, defaultVal);
        return cell->data;
    }

    //----------------- PRIVATE -----------------//

    internal::StorageCell* GameSave::getCell(const GameSaveSlot id)
    {
        for (auto& cell : storage)
        {
            if (cell.id == id)
            {
                return &cell;
            }
        }
        return nullptr;
    }

    internal::StorageCell* GameSave::getCellOrNew(GameSaveSlot id, StorageType type)
    {
        auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return &storage.emplace_back(id, type);
        }
        cell->type = type;
        return cell;
    }

    void GameSave::assignDataImpl(const GameSaveSlot id, const void* data, const int bytes, const StorageType type)
    {
        auto* cell = getCellOrNew(id, type);
        cell->data.resize(bytes);
        std::memcpy(cell->data.data(), data, bytes);
    }

} // namespace magique
