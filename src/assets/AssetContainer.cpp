#include <algorithm>
#include <cstring>

#include <magique/assets/container/AssetContainer.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>
#include <raylib/raylib.h>
#include <cxutil/cxstring.h>

struct Sorter
{
    static bool Full(const char* a, const char* b)
    {
        auto [baseA, baseALength, numA] = SplitPathAndNumber(a);
        auto [baseB, baseBLength, numB] = SplitPathAndNumber(b);

        int basePathComparison = std::strncmp(baseA, baseB, std::min(baseALength, baseBLength));
        if (basePathComparison != 0)
        {
            return basePathComparison < 0;
        }

        if (numA != numB)
        {
            return numA < numB;
        }

        return *a > *b;
    }
    static bool Directory(const char* a, const char* b, int bSize) { return strncmp(a, b, bSize) < 0; }

private:
    static bool isDigit(const char c) { return c >= '0' && c <= '9'; }
    static std::tuple<const char*, size_t, long long> SplitPathAndNumber(const char* path)
    {
        const char* p = path;
        const char* lastDot = nullptr;
        const char* numStart = nullptr;
        while (*p)
        {
            if (*p == '.')
                lastDot = p;
            p++;
        }
        if (lastDot)
        {
            numStart = lastDot;
            while (numStart > path && isDigit(*(numStart - 1)))
            {
                --numStart;
            }
        }

        long long number = -1;
        if (numStart && numStart < lastDot)
        {
            number = 0;
            for (const char* n = numStart; n < lastDot; ++n)
            {
                number = number * 10 + (*n - '0');
            }
        }

        size_t basePathLength = numStart ? numStart - path : (lastDot ? lastDot - path : p - path);
        return {path, basePathLength, number};
    }
};

int FindAssetPos(const std::vector<magique::Asset>& assets, const char* name)
{
    int low = 0;
    int high = static_cast<int>(assets.size());

    while (low <= high)
    {
        const int mid = low + (high - low) / 2;

        if (strcmp(assets[mid].path, name) == 0) [[unlikely]]
            return mid;

        if (Sorter::Full(assets[mid].path, name))
            low = mid + 1;
        else
            high = mid - 1;
    }

    // If we reach here, then element was not present
    return -1;
}

int FindDirectoryPos(const std::vector<magique::Asset>& assets, const char* name, const int size)
{
    int low = 0;
    int high = static_cast<int>(assets.size());

    while (low <= high)
    {
        const int mid = low + (high - low) / 2;

        if (strncmp(assets[mid].path, name, size) == 0) [[unlikely]]
            return mid;

        if (Sorter::Directory(assets[mid].path, name, size))
            low = mid + 1;
        else
            high = mid - 1;
    }

    // If we reach here, then element was not present
    return -1;
}

namespace magique
{

    bool Asset::hasExtension(const char* extension) const
    {
        M_ASSERT(extension != nullptr, "Passing nullptr");
        const auto* ext = GetFileExtension(path);
        if (ext == nullptr)
            return false;
        return strcmp(extension, ext);
    }

    const char* Asset::getFileName(bool extension) const
    {
        const char* workPtr = path;
        const char* lastSep = nullptr;
        int len = 0;
        while (*workPtr)
        {
            if (lastSep)
            {
                if (!extension && *workPtr == '.')
                    break;
                len++;
            }

            if (*workPtr == '/')
            {
                len = 0;
                lastSep = workPtr + 1;
            }
            workPtr++;
        }

        if (lastSep == nullptr)
            return nullptr;


        memcpy(stringBuffer, lastSep, std::min(64, len));
        stringBuffer[std::min(64, len)] = '\0';
        return stringBuffer;
    }

    const char* Asset::getExtension() const
    {
        const char* workPtr = path;
        const char* lastDot = nullptr;

        while (*workPtr)
        {
            if (*workPtr == '.')
            {
                lastDot = workPtr;
            }
            workPtr++;
        }

        if (lastDot == nullptr)
            return nullptr;

        int len = 0;
        workPtr = lastDot;
        while (*workPtr)
        {
            len++;
            workPtr++;
        }

        memcpy(stringBuffer, lastDot, std::min(64, len));
        stringBuffer[std::min(64, len)] = '\0';
        return stringBuffer;
    }

    //----------------- CONTAINER -----------------//

    AssetContainer::AssetContainer(const char* nativeData, std::vector<Asset>&& newAssets) : nativeData(nativeData)
    {
        if (!assets.empty())
        {
            LOG_WARNING("Trying to load with image into a non-empty container");
            // This isnt necessarily bad - but is probably a mistake
            delete[] nativeData;
            assets.clear();
        }

        assets = std::move(newAssets);

        // This sorts all entries after directory and then insdie a directory after numbering
        std::ranges::sort(assets, [](const Asset& a1, const Asset& a2) { return Sorter::Full(a1.path, a2.path); });
    }

    AssetContainer& AssetContainer::operator=(AssetContainer&& container) noexcept
    {
        if (this == &container)
            return *this;

        assets = std::move(container.assets);
        nativeData = container.nativeData;
        container.nativeData = nullptr;

        return *this;
    }
    AssetContainer::~AssetContainer() { delete[] nativeData; }

    void AssetContainer::iterateDirectory(const char* name, const std::function<void(const Asset&)>& func) const
    {
        M_ASSERT(name != nullptr, "Passing nullptr!");
        M_ASSERT(!assets.empty(), "No assets loaded!");

        const int size = static_cast<int>(strlen(name));
        int pos = FindDirectoryPos(assets, name, size);


        if (pos == -1) [[unlikely]]
        {
            LOG_WARNING("No directory with name %s found!", name);
            return;
        }

        while (pos > 0 && strncmp(assets[pos - 1].path, name, size) == 0)
        {
            pos--;
        }

        do
        {
            func(assets[pos]);
            pos++;
        }
        while (pos < assets.size() && strncmp(assets[pos].path, name, size) == 0);
    }

    const Asset& AssetContainer::getAssetByPath(const char* name) const
    {
        M_ASSERT(name != nullptr, "Passing nullptr!");
        M_ASSERT(!assets.empty(), "No assets loaded!");

        const int pos = FindAssetPos(assets, name);

        if (pos == -1) [[unlikely]]
        {
            LOG_ERROR("No asset with name %s found! Returning empty asset", name);
            return Asset();
        }

        return assets[pos];
    }

    const Asset& AssetContainer::getAsset(const char* name) const
    {
        M_ASSERT(name != nullptr, "Passing nullptr!");
        M_ASSERT(!assets.empty(), "No assets loaded!");

        for (const auto& a : assets)
        {
            if (cxstructs::str_cmp_rev(a.path, name) == 0)
                return a;
        }

        LOG_ERROR("No asset with name %s found! Returning empty asset", name);
        return Asset();
    }


} // namespace magique