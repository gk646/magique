#include <algorithm>
#include <magique/persistence/container/GameSave.h>

#include <cxutil/cxio.h>

namespace magique
{
    bool ContainsCell(const StorageID id, const std::vector<StorageCell>& storage)
    {
        return std::ranges::binary_search(storage, StorageCell{id, nullptr, 0}, StorageCell::operator<);
    }

    SaveGame::SaveGame(const char* data, int size) {}

    bool SaveGame::saveData(StorageID id, void* data, int size)
    {
        if (std::ranges::contains(storage, StorageCell{id, nullptr, 0}))
            return false;
        StorageCell cell{id, static_cast<const char*>(data), size};
        storage.push_back(cell);
        return true;
    }

    const StorageCell* SaveGame::getData(const StorageID id) const
    {
        const auto it = std::lower_bound(storage.begin(), storage.end(), StorageCell{id, nullptr, 0});
        if (it != storage.end() && it->id == id)
        {
            return &(*it);
        }
        return nullptr;
    }

} // namespace magique