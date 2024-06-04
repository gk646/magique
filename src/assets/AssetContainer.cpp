#include <algorithm>
#include <string>

#include <magique/assets/container/AssetContainer.h>
#include <magique/util/Logging.h>


struct NumericSort
{
    static bool Comp(const char* a, const char* b)
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

        return a < b;
    }

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


namespace magique
{
    AssetContainer::AssetContainer(std::vector<Asset>&& newAssets)
    {
        if (!assets.empty())
        {
            LOG_WARNING("Trying to load with image into a non-empty container");
            // This isnt necessarily bad - but is probably a mistake
            for (auto& a : assets)
            {
                delete[] a.data;
                a.data = nullptr;
            }
            assets.clear();
        }

        assets = std::move(newAssets);

        // This sorts all entries after directory and then insdie a directory after numbering
        std::ranges::sort(assets, [](const Asset& a1, const Asset& a2) { return NumericSort::Comp(a1.name, a2.name); });
    }


    AssetContainer::~AssetContainer()
    {
        for (auto& a : assets)
        {
            delete[] a.data;
            a.data = nullptr;
        }
    }

    bool AssetContainer::IterateDirectory(const char* name, LoadFunc func) const { return true; }

    const Asset& AssetContainer::GetAsset(const char* name) const
    {

        for (const auto& a : assets)
        {
            if (strcmp(a.name, name) == 0)
                return a;
        }

        LOG_ERROR("No asset with name %s found! Returning empty asset", name);
        return Asset();
    }


} // namespace magique