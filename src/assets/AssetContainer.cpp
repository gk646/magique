// SPDX-License-Identifier: zlib-acknowledgement
#include <functional>
#include <cstring>

#include <magique/assets/container/AssetContainer.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

#include "internal/utils/STLUtil.h"

struct Sorter
{
    static bool Full(const char* a, const char* b)
    {
        auto [baseA, baseALength, numA] = SplitPathAndNumber(a);
        auto [baseB, baseBLength, numB] = SplitPathAndNumber(b);

        int basePathComparison = strncmp(baseA, baseB, std::min(baseALength, baseBLength));
        if (basePathComparison == 0 && baseALength != baseBLength)
        {
            basePathComparison = baseALength < baseBLength ? -1 : 1;
        }
        if (basePathComparison != 0)
        {
            return basePathComparison < 0;
        }

        if (numA != numB)
        {
            return numA < numB;
        }

        return strcmp(a, b) < 0;
    }
    static bool Directory(const char* a, const char* b, const int bSize) { return strncmp(a, b, bSize) < 0; }

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
    static constexpr Asset emptyAsset{};

    AssetContainer::~AssetContainer() { delete[] nativeData; }

    void AssetContainer::sort()
    {
        auto comparator = [](const Asset& a1, const Asset& a2) { return Sorter::Full(a1.path, a2.path); };
        // This sorts all entries after directory and then inside a directory after numbering
        //std::ranges::sort(assets, comparator);
        QuickSort(assets.data(), static_cast<int>(assets.size()), comparator);
    }

    void AssetContainer::iterateDirectory(const char* name, const std::function<void(Asset)>& func) const
    {
        MAGIQUE_ASSERT(name != nullptr, "Passing nullptr!");
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
        while (pos < static_cast<int>(assets.size()) && strncmp(assets[pos].path, name, size) == 0);
    }

    const Asset& AssetContainer::getAssetByPath(const char* name) const
    {
        MAGIQUE_ASSERT(name != nullptr, "Passing nullptr!");
        MAGIQUE_ASSERT(!assets.empty(), "No assets loaded!");

        const int pos = FindAssetPos(assets, name);

        if (pos == -1) [[unlikely]]
        {
            LOG_ERROR("No asset with name %s found! Returning empty asset", name);
            return emptyAsset;
        }

        return assets[pos];
    }

    const Asset& AssetContainer::getAsset(const char* name) const
    {
        MAGIQUE_ASSERT(name != nullptr, "Passing nullptr!");
        MAGIQUE_ASSERT(!assets.empty(), "No assets loaded!");
        for (const auto& asset : assets)
        {
            if (asset.endsWith(name))
                return asset;
        }
        LOG_ERROR("No asset with name %s found! Returning empty asset", name);
        return emptyAsset;
    }

    const std::vector<Asset>& AssetContainer::getAllAssets() const { return assets; }

    int AssetContainer::getSize() const { return static_cast<int>(assets.size()); }

} // namespace magique