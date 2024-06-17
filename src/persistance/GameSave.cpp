#include <algorithm>
#include <filesystem>

#include <magique/persistence/container/GameSave.h>
#include <magique/util/Logging.h>

#include <cxutil/cxio.h>

namespace magique
{
    constexpr auto* FILE_HEADER = "MAGIQUE_SAVE_FILE";

    bool ContainsCell(const StorageID id, const std::vector<StorageCell>& storage)
    {
        return std::ranges::binary_search(storage, StorageCell{id, nullptr, 0}, &StorageCell::operator<);
    }

    bool SizeCheck(const int currenSize, const int totalSize)
    {
        if (currenSize > totalSize)
        {
            LOG_ERROR("Error reading save data");
            return false;
        }
        return true;
    }

    GameSave::~GameSave()
    {
        if (!isPersisted)
        {
            LOG_WARNING("GameSave is deleted without being saved!");
        }
    }

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
        int32_t totalEntries = static_cast<int32_t>(save.storage.size());
        fwrite(&totalEntries, sizeof(int32_t), 1, file);

        int32_t dataSize = 0;

        for (const auto& [id, data, size] : save.storage)
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


    bool GameSave::saveData(const StorageID id, const char* data, const int typeSize, const int count)
    {
        const auto cell = getData(id);

        // Copy the data
        const int totalSize = count * typeSize;
        const auto saveData = new char[totalSize];
        memcpy(saveData, data, totalSize);

        // Doesnt exist yet
        if (cell == nullptr)
        {
            storage.push_back({id, static_cast<char*>(saveData), totalSize});
        }
        else // Overwrite existing data
        {
            delete[] cell->data;
            cell->data = static_cast<char*>(saveData);
            cell->size = totalSize;
        }
        return true;
    }


    StorageCell* GameSave::getData(const StorageID id)
    {
        for (auto& cell : storage)
        {
            if (cell.id == id)
            {
                return &cell;
            }
        }
        return nullptr;
        const auto it = std::lower_bound(storage.begin(), storage.end(), StorageCell{id, nullptr, 0});
        if (it != storage.end() && it->id == id)
        {
            return &*it;
        }
        return nullptr;
    }

} // namespace magique