#define _CRT_SECURE_NO_WARNINGS

#include <magique/util/Logging.h>
#include <magique/persistence/container/GameConfig.h>

#include "internal/headers/Security.h"

inline constexpr auto FILE_HEADER = "CONFIG";

namespace magique
{
    void GameConfig::SaveToFile(const GameConfig& config, const char* filePath, const uint64_t encryptionKey)
    {
        int totalSize = 0;
        for (const auto& cell : config.storage)
        {
            totalSize += 1; // Type
            totalSize += 4; // ID
            if (cell.type == StorageType::STRING)
            {
                totalSize += strlen(cell.string); // String
            }
            else
            {
                totalSize += 8; // Data
            }
        }

        // Generate data
        auto* data = new unsigned char[totalSize];

        int idx = 0;
        for (const auto& cell : config.storage)
        {
            auto type = static_cast<uint8_t>(cell.type); // only 5 types - but type int cause its in user space
            std::memcpy(&data[idx], &type, sizeof(uint8_t));
            ++idx;
            std::memcpy(&data[idx], &cell.id, sizeof(int));
            idx += 4;

            if (cell.type == StorageType::STRING)
            {
                const auto len = strlen(cell.string); // String
                std::memcpy(&data[idx], cell.string, len);
                idx += len;
            }
            else
            {
                std::memcpy(&data[idx], cell.buffer, 8);
                idx += 8;
            }
        }

        SymmetricEncrypt((char*)data, totalSize, encryptionKey);

        FILE* file = fopen(filePath, "wb");
        if (file == nullptr)
        {
            LOG_ERROR("Failed to open file for writing: %s", filePath);
            delete[] data;
            return;
        }
        setvbuf(file, nullptr, _IONBF, 0);

        // Header
        fwrite(FILE_HEADER, strlen(FILE_HEADER), 1, file);
        // Data
        fwrite(data, totalSize, 1, file);

        fclose(file);
        delete[] data;

        LOG_INFO("Successfully saved gamesave: %s | Entires: %d", filePath, static_cast<int>(config.storage.size()));
    }

    GameConfig GameConfig::LoadFromFile(const char* filePath, const uint64_t encryptionKey)
    {
        GameConfig config;
        FILE* file = fopen(filePath, "rb");
        if (file == nullptr)
        {
            LOG_ERROR("File does not exist: %s", filePath);
            return config;
        }
        auto constexpr headerSize = static_cast<int>(std::char_traits<char>::length(FILE_HEADER));

        fseek(file, 0, SEEK_END);
        const int totalSize = static_cast<int>(ftell(file)) - headerSize;
        fseek(file, 0, SEEK_SET);

        // Check header first
        char buffer[headerSize];
        fread(buffer, headerSize, 1, file);
        if (memcmp(FILE_HEADER, buffer, headerSize) != 0)
        {
            LOG_ERROR("Malformed gameconfig file: %s", filePath);
            fclose(file);
            return config;
        }

        auto* data = new char[totalSize];
        fread(data, totalSize, 1, file);

        // Decrypt
        SymmetricEncrypt(data, totalSize, encryptionKey);

        int i = 0;
        while (i < totalSize)
        {
            uint8_t type = 0;
            std::memcpy(&type, &data[i], sizeof(uint8_t));
            ++i;

            int storageID = 0;
            std::memcpy(&storageID, &data[i], sizeof(int));
            i += 4;

            GameConfigStorageCell cell{static_cast<ConfigID>(storageID)};

            if (type == static_cast<uint8_t>(StorageType::STRING))
            {
                const int len = strlen(&data[i]); // thats funny
                cell.assign(&data[i], len, static_cast<StorageType>(type));
                i += len;
            }
            else
            {
                cell.assign(&data[i], 8, static_cast<StorageType>(type));
                i += 8;
            }
            config.storage.push_back(cell);
        }

        fclose(file);
        delete[] data;
        LOG_INFO("Successfully loaded gamesave: %s | Entries: %d", filePath, static_cast<int>(config.storage.size()));
        return config;
    }

    GameConfigStorageCell* GameConfig::getCell(const ConfigID id)
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

    void GameConfig::saveKeybind(const ConfigID id, const Keybind keybind)
    {
        auto* cell = getCell(id);
        if (cell == nullptr)
        {
            GameConfigStorageCell newCell{id};
            newCell.assign(nullptr, 0, StorageType::KEY_BIND, keybind);
            storage.push_back(newCell);
        }
        else
        {
            cell->assign(nullptr, 0, StorageType::KEY_BIND, keybind);
        }
    }

    void GameConfig::saveString(const ConfigID id, const std::string& string)
    {
        auto* cell = getCell(id);
        if (cell == nullptr)
        {
            GameConfigStorageCell newCell{id};
            newCell.assign(string.data(), static_cast<int>(string.size()), StorageType::STRING);
            storage.push_back(newCell);
        }
        else
        {
            cell->assign(string.data(), static_cast<int>(string.size()), StorageType::STRING);
        }
    }

    Keybind GameConfig::getKeybindOrElse(const ConfigID id, const Keybind defaultKeybind)
    {
        const auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return defaultKeybind;
        }
        if (cell->type != StorageType::KEY_BIND)
        {
            return defaultKeybind;
        }
        return cell->keybind;
    }

    std::string GameConfig::getStringOrElse(const ConfigID id, const std::string& defaultKey)
    {
        const auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return defaultKey;
        }
        if (cell->type != StorageType::STRING)
        {
            return defaultKey;
        }
        return {cell->string}; // is null terminated
    }
} // namespace magique