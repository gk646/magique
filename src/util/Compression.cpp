#include <magique/util/Compression.h>
#include <magique/internal/DataStructures.h>
#include <magique/util/Logging.h>

namespace magique
{
    using BytePointer = unsigned char*;
    using PatternType = uint64_t;
    using MarkerType = uint8_t;
    using PatternMap = HashMap<PatternType, int>;
    using PatternVec = std::vector<std::pair<PatternType, int>>;

    bool HasOverlap(const PatternType a, const PatternType b)
    {
        uint64_t mask = 0xFF;
        for (int i = 1; i < sizeof(PatternType); ++i)
        {
            if (a >> i * 8 == (b & ~(mask << i * 8)) || b >> i * 8 == (a & ~(mask << i * 8)))
            {
                return true;
            }
            mask = mask << 8 | 0xFF;
        }
        return false;
    }

    void GeneratePatterns(BytePointer data, int size, PatternMap& map)
    {
        int startByte = 0;
        while (startByte <= size - sizeof(PatternType))
        {
            PatternType tempPattern;
            std::memcpy(&tempPattern, data + startByte, sizeof(PatternType));
            auto it = map.find(tempPattern);
            if (it != map.end())
            {
                it->second++;
            }
            else
            {
                map.insert({tempPattern, 1});
            }
            startByte++;
        }
    }

    void SortPatterns(PatternVec& topPatterns, PatternMap& map, const int maxSize)
    {
        auto predicate = [](const auto& a, const auto& b)
        { return a.second > b.second || (a.second == b.second && a.first < b.first); };

        for (const auto& pair : map)
        {
            const auto it = std::lower_bound(topPatterns.begin(), topPatterns.end(), pair, predicate);
            if (it != topPatterns.end() || topPatterns.size() < maxSize)
            {
                topPatterns.insert(it, pair);

                if (topPatterns.size() > maxSize)
                {
                    topPatterns.pop_back();
                }
            }
        }
    }

    void FilterPatterns(PatternVec& patterns)
    {
        for (auto it = patterns.begin(); it != patterns.end();)
        {
            bool keep = true;
            for (auto fit = patterns.begin(); fit != patterns.end();)
            {
                if (it != fit && HasOverlap(it->first, fit->first))
                {
                    if (it->second > fit->second)
                    {
                        fit = patterns.erase(fit);
                    }
                    else
                    {
                        keep = false;
                        break;
                    }
                }
                else
                {
                    ++fit;
                }
            }
            if (!keep)
            {
                it = patterns.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    int CalculateNewSize(int size, const PatternVec& vec, int markers)
    {
        for (const auto& pattern : vec)
        {
            if (markers <= 0)
                break;
            size -= pattern.second * (sizeof(PatternType) - sizeof(MarkerType));
            markers--;
        }
        return size;
    }

    std::vector<MarkerType> FindMarkers(BytePointer data, const int size)
    {
        std::vector bytePresence(256, false);
        std::vector<MarkerType> uniqueMarkers;

        for (int i = 0; i < size; ++i)
        {
            bytePresence[data[i]] = true;
        }

        for (int i = 0; i < 256; ++i)
        {
            if (!bytePresence[i])
            {
                uniqueMarkers.push_back(static_cast<MarkerType>(i));
            }
        }

        return uniqueMarkers;
    }

    DataPointer<const char> GenerateCompressedData(BytePointer data, int size, const PatternVec& vec)
    {
        const std::vector<MarkerType> uniqueMarkers = FindMarkers(data, size);

        if (uniqueMarkers.empty()) // No possible marker symbols
            return {reinterpret_cast<const char*>(data), size};

        printf("unique: %zu\n", uniqueMarkers.size());

        const int newSize = CalculateNewSize(size, vec, uniqueMarkers.size());
        auto* compressedData = new unsigned char[newSize * 2];

        HashMap<PatternType, MarkerType> markerMap;
        for (size_t i = 0; i < vec.size() && i < uniqueMarkers.size(); ++i)
        {
            markerMap[vec[i].first] = uniqueMarkers[i];
        }

        int compressedIndex = 0;
        int i = 0;
        int count = 0;
        while (i <= size - sizeof(PatternType))
        {
            PatternType tempPattern;
            std::memcpy(&tempPattern, data + i, sizeof(PatternType));
            auto it = markerMap.find(tempPattern);
            if (it != markerMap.end())
            {
                count++;
                compressedData[compressedIndex++] = it->second;
                i += sizeof(PatternType);
                if (count % 1000 == 0)
                    printf("Count: %d\n", count);
            }
            else
            {
                compressedData[compressedIndex++] = data[i++];
            }
        }

        // Copy any remaining bytes that didn't match a pattern
        while (i < size)
        {
            compressedData[compressedIndex++] = data[i++];
        }

        return {reinterpret_cast<const char*>(compressedData), compressedIndex};
    }


    DataPointer<const char> Compress(const char* in, const int size)
    {
        const auto* data = reinterpret_cast<const unsigned char*>(in);
        if (data == nullptr || size <= 0)
        {
            LOG_ERROR("Passed nullptr or invalid size: %d", size);
            return {nullptr, 0};
        }
        PatternMap patternMap;
        HashSet<int> patternPos;
        PatternVec topPatterns;
        topPatterns.reserve(UINT8_MAX + 1);

        GeneratePatterns((BytePointer)data, size, patternMap);
        printf("Size %d\n", patternMap.size());

        SortPatterns(topPatterns, patternMap, UINT8_MAX);

        FilterPatterns(topPatterns);
        printf("Size %d\n", topPatterns.size());

        for (const auto& pair : topPatterns)
        {
            printf("Pattern: %016llx, Count: %d\n", pair.first, pair.second);
        }

        return GenerateCompressedData((BytePointer)data, size, topPatterns);
    }


    DataPointer<const char> DeCompress(const char* data, int size) { return {nullptr, 0}; }


} // namespace magique