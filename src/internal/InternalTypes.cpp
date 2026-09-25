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
            LOG_WARNING("GameStorage is not empty and destroyed without usage!");
        }
    }

    bool StorageContainer::ToFile(StorageContainer& container, std::string_view path, EncryptionKey key)
    {
        std::string buffer;
        JSONExport(container.cells, buffer);
        buffer = std::move(DataCompressImpl(buffer));

        DataEncrypt(buffer, key);
        if (!DataWriteFile(path, buffer))
            return false;

        LOG_INFO("Saved GameStorage: %s | Size: %.2fkb", path.data(), (float)buffer.size() / 1000.0F);
        container.isSaved = true;
        return true;
    }

    bool StorageContainer::FromFile(StorageContainer& container, std::string_view path, EncryptionKey key)
    {
        MAGIQUE_ASSERT(container.isLoaded == false, "Can only load from empty save!");
        MAGIQUE_ASSERT(container.cells.empty(), "Can only load from empty save!");
        container.isLoaded = true;

        std::string buffer;
        if (!DataReadFile(path, buffer))
            return false;

        auto result = DataDecompressImpl(buffer);
        if (!result)
        {
            LOG_WARNING("Failed to decompress asset pack: %s", path.data());
            return false;
        }
        buffer = std::move(result.value().get());

        DataDecrypt(buffer, key);
        JSONImport(buffer, container.cells);

        LOG_INFO("Loaded GameStorage: %s | Size: %.2fkb", path.data(), (float)buffer.size() / 1000.0F);
        return true;
    }

    bool StorageContainer::eraseImpl(std::string_view slot)
    {
        return std::erase_if(cells, [&](const auto& cell) { return cell.name == slot; }) > 0;
    }

} // namespace magique::internal
