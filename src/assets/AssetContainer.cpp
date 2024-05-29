#include <magique/assets/container/AssetContainer.h>


#include <string>

struct NumericSort
{
    static bool isDigit(char c) { return c >= '0' && c <= '9'; }
    static std::tuple<const char*, size_t, long long> splitPathAndNumber(const char* path);
    bool operator()(const std::string& a, const std::string& b) const;
};
std::tuple<const char*, size_t, long long> NumericSort::splitPathAndNumber(const char* path)
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
bool NumericSort::operator()(const std::string& a, const std::string& b) const
{
    auto [baseA, baseALength, numA] = splitPathAndNumber(a.c_str());
    auto [baseB, baseBLength, numB] = splitPathAndNumber(b.c_str());

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

namespace magique
{
    AssetContainer::AssetContainer() {}


    AssetContainer::~AssetContainer()
    {
        for (auto& a : assets)
        {
            delete[] a.data;
            a.data = nullptr;
        }
    }


    void AssetContainer::sort() {}
} // namespace magique