// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/internal/InternalTypes.h>
#include <magique/assets/JSON.h>
#include <magique/util/Data.h>

namespace magique::internal
{
    StorageContainer::~StorageContainer()
    {
        if (!cells.empty() && !(isLoaded || isSaved))
        {
            LOG_WARNING("GameSave/GameConfig is not empty and destroyed without usage!");
        }
    }

    bool StorageContainer::ToFile(StorageContainer& container, std::string_view path, std::string_view name,
                                  EncryptionKey key)
    {
        std::string buffer;
        JSONExport(container.cells, buffer);
        DataEncrypt(buffer, key);

        FILE* file = fopen(path.data(), "wb");
        if (file == nullptr)
        {
            LOG_ERROR("Failed to open file for writing: %s", path.data());
            return false;
        }

        setvbuf(file, nullptr, _IONBF, 0);
        fwrite(buffer.data(), buffer.size(), 1, file);
        fclose(file);
        LOG_INFO("Saved %s: %s | Size: %.2fkb", name.data(), path.data(), (float)buffer.size() / 1000.0F);
        container.isSaved = true;
        return true;
    }

    bool StorageContainer::FromFile(StorageContainer& container, std::string_view path, std::string_view name,
                                    EncryptionKey key)
    {
        MAGIQUE_ASSERT(container.isLoaded == false, "Can only load from empty save!");
        MAGIQUE_ASSERT(container.cells.empty(), "Can only load from empty save!");
        container.isLoaded = true;

        FILE* file = fopen(path.data(), "rb");
        if (file == nullptr)
        {
            LOG_WARNING("File does not exist. Will be created once you save: %s", path.data());
            return false;
        }

        fseek(file, 0, SEEK_END);
        const auto totalSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        std::string buffer(totalSize, '\0');
        fread(buffer.data(), totalSize, 1, file);
        fclose(file);

        DataDecrypt(buffer, key);
        JSONImport(buffer, container.cells);

        LOG_INFO("Loaded %s: %s | Size: %.2fkb", name.data(), path.data(), static_cast<float>(totalSize) / 1000.0F);
        return true;
    }

    void StorageContainer::eraseImpl(std::string_view slot)
    {
        std::erase_if(cells, [&](const auto& cell) { return cell.name == slot; });
    }

} // namespace magique::internal
