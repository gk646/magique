#include <algorithm>

#include <magique/persistence/container/GameSave.h>
#include <magique/util/Logging.h>

using StorageCell = magique::GameSaveStorageCell;

namespace magique
{
    constexpr auto* FILE_HEADER = "MAGIQUE_SAVE_FILE";

    GameSave::GameSave(const GameSave& other) : isPersisted(other.isPersisted), storage(other.storage) {}

    GameSave::GameSave(GameSave&& other) noexcept : isPersisted(other.isPersisted), storage(std::move(other.storage)) {}

    GameSave::~GameSave()
    {
        if (!isPersisted)
        {
            LOG_WARNING("GameSave is deleted without being saved!");
        }
    }

    //----------------- PERSISTENCE -----------------//

    GameSave GameSave::LoadGameSave(const char* filePath, uint64_t encryptionKey)
    {
        char buffer[24];
        GameSave save{};

        FILE* file = fopen(filePath, "rb");
        if (!file)
        {
            LOG_ERROR("File does not exist | Will be created when saving: %s", filePath);
            return save;
        }

        int32_t totalSize = 0;
        int32_t totalEntries = 0;

        // Header
        fread(buffer, strlen(FILE_HEADER), 1, file);
        if (memcmp(FILE_HEADER, buffer, strlen(FILE_HEADER)) != 0)
        {
            LOG_ERROR("Malformed save file: %s", filePath);
            fclose(file);
            return save;
        }

        // Saved size
        fread(&totalSize, sizeof(int32_t), 1, file);

        // Saved entries
        fread(&totalEntries, sizeof(int32_t), 1, file);
        if (totalEntries < 0 || totalEntries > 50'000)
        { // Sanity check
            LOG_ERROR("Invalid number of entries in save file: %s", filePath);
            fclose(file);
            return save;
        }
        save.storage.reserve(totalEntries);

        while (ftell(file) < totalSize)
        {
            int id;
            fread(&id, sizeof(int), 1, file);

            int size;
            fread(&size, sizeof(int), 1, file);

            auto* data = new char[size];
            fread(data, 1, size, file);

            save.storage.push_back(StorageCell{static_cast<StorageID>(id), data, size});
        }
        fclose(file);
        return save;
    }

    bool GameSave::SaveGameSave(GameSave& save, const char* filePath, uint64_t encryptionKey)
    {
        FILE* file = fopen(filePath, "wb");
        if (!file)
        {
            LOG_ERROR("Failed to open file for writing: %s", filePath);
            return false;
        }

        // Header
        fwrite(FILE_HEADER, 1, strlen(FILE_HEADER), file);

        // Total size
        int32_t totalSize = 0;
        fwrite(&totalSize, sizeof(int32_t), 1, file);

        // Total entries
        const auto totalEntries = static_cast<int32_t>(save.storage.size());
        fwrite(&totalEntries, sizeof(int32_t), 1, file);

        int32_t dataSize = 0;

        for (const auto& [id, data, size, _] : save.storage)
        {
            fwrite(&id, sizeof(int), 1, file);
            fwrite(&size, sizeof(int), 1, file);
            fwrite(data, 1, size, file);
            dataSize += sizeof(int) * 2 + size;
        }

        // Calculate and write the total size at the beginning of the file
        totalSize = strlen(FILE_HEADER) + sizeof(int32_t) * 2 + dataSize;
        fseek(file, strlen(FILE_HEADER), SEEK_SET);
        fwrite(&totalSize, sizeof(int32_t), 1, file);

        fclose(file);
        save.isPersisted = true;
        return true;
    }

    //----------------- SAVING -----------------//

    void GameSave::saveString(const StorageID id, const std::string& string)
    {
        auto* cell = getCell(id);
        if (cell == nullptr) // Doesnt exist
        {
            const auto stringIdx = static_cast<int>(stringStorage.size());
            storage.push_back({nullptr, id, stringIdx, 0, StorageCell::Type::STRING});
            stringStorage.emplace_back(string);
        }
        else // Exists
        {
            if (cell->type == StorageType::STRING) // Its a string
            {
                stringStorage[cell->size] = string; // Just replace the old string for this cell
            }
            else // Its something else - delete and add new string
            {
                cell->free();
                const auto stringIdx = static_cast<int>(stringStorage.size());
                cell->size = stringIdx;
                cell->type = StorageType::STRING;
                stringStorage.emplace_back(string);
            }
        }
    }

    //----------------- GETTING -----------------//

    std::string GameSave::getStringOrElse(const StorageID id, const std::string& defaultVal)
    {
        const auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return defaultVal;
        }
        if (cell->type == StorageType::STRING)
        {
            return stringStorage[cell->size];
        }
        return defaultVal;
    }


    //----------------- PRIVATE -----------------//

    GameSaveStorageCell* GameSave::getCell(const StorageID id)
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

    void GameSave::assignDataImpl(const StorageID id, const char* data, const int bytes)
    {
        auto cell = getCell(id);
        if (cell == nullptr)
        {
            cell = &storage.emplace_back(id, nullptr, 0, 0);
        }
        cell->assign(data, bytes);
    }

} // namespace magique