#include <magique/util/Compression.h>
#include <magique/internal/DataStructures.h>
#include <magique/util/Logging.h>

namespace magique
{
    using BytePointer = unsigned char*;
    using PatternType = uint32_t;
    using MarkerType = uint8_t;
    using PatternMap = HashMap<PatternType, int>;
    using PatternVec = std::vector<std::pair<PatternType, int>>;
    using MarkerVec = std::vector<MarkerType>;
    using Marker2Vec = std::vector<uint16_t>;

    bool HasOverlap(const PatternType ap, const PatternType bp)
    {
        constexpr int patternSize = sizeof(PatternType);
        // Convert PatternType to byte arrays for easier comparison
        const uint8_t* a = reinterpret_cast<const uint8_t*>(&ap);
        const uint8_t* b = reinterpret_cast<const uint8_t*>(&bp);

        for (int i = 1; i < patternSize; ++i)
        {
            bool overlap = true;
            for (int j = 0; j < patternSize - i; ++j)
            {
                if (a[i + j] != b[j])
                {
                    overlap = false;
                    break;
                }
            }
            if (overlap)
            {
                return true;
            }
        }

        // Check for suffix of b overlapping with prefix of a
        for (int i = 1; i < patternSize; ++i)
        {
            bool overlap = true;
            for (int j = 0; j < patternSize - i; ++j)
            {
                if (b[i + j] != a[j])
                {
                    overlap = false;
                    break;
                }
            }
            if (overlap)
            {
                return true;
            }
        }

        return false;
    }

    void GeneratePatterns(BytePointer data, int size, PatternMap& map)
    {
        HashMap<PatternType, uint8_t> patternLock;
        int startByte = 0;
        while (startByte <= size - sizeof(PatternType))
        {
            PatternType tempPattern;
            std::memcpy(&tempPattern, data + startByte, sizeof(PatternType));

            if (!patternLock.contains(tempPattern))
            {
                map[tempPattern]++;
                patternLock.insert({tempPattern, sizeof(PatternType)});
            }

            for (auto delIt = patternLock.begin(); delIt != patternLock.end();)
            {
                delIt->second--;
                if (delIt->second == 0) [[likely]]
                {
                    delIt = patternLock.erase(delIt);
                }
                else
                {
                    ++delIt;
                }
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
                //topPatterns.insert({it, pair});

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

    int CalculateNewSize(int size, const PatternVec& vec, int markers, int markers2)
    {
        //size += 5; // Tag
        for (const auto& pattern : vec)
        {
            if (markers > 0)
            {
                size -= pattern.second * (sizeof(PatternType) - sizeof(MarkerType));
                markers--;
            }
            else if (markers2 > 0)
            {
                size -= pattern.second * (sizeof(PatternType) - sizeof(uint16_t));
                markers2--;
            }
        }
        return size;
    }

    std::pair<MarkerVec, Marker2Vec> FindMarkers(BytePointer data, const int size)
    {
        std::vector bytePresence(256, false);
        MarkerVec uniqueMarkers;
        Marker2Vec unique2Markers;

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

        if (uniqueMarkers.size() < 25)
        {
            HashSet<uint16_t> uniqueBytes;
            for (int i = 0; i < size - 1; ++i)
            {
                uint16_t marker = (data[i] << 8) | data[i + 1];
                uniqueBytes.insert(marker);
                if (uniqueBytes.size() >= UINT16_MAX)
                    break;
            }

            if (uniqueBytes.size() < UINT16_MAX)
            {
                for (uint16_t i = UINT8_MAX; i < UINT16_MAX; ++i)
                {
                    if (uniqueBytes.find(i) == uniqueBytes.end())
                    {
                        unique2Markers.push_back(i);
                    }
                }
            }
        }
        return {uniqueMarkers, unique2Markers};
    }

    DataPointer<const char> GenerateCompressedData(BytePointer data, int size, const PatternVec& vec)
    {
        const auto [markers, markers2] = FindMarkers(data, size);

        int unique = static_cast<int>(markers.size());
        int unique2 = static_cast<int>(markers2.size());

        if (unique == 0 && unique2 == 0) // No possible marker symbols
            return {reinterpret_cast<const char*>(data), size};

        printf("unique: %zu\n", markers.size());
        printf("unique2: %zu\n", markers2.size());

        const int newSize = CalculateNewSize(size, vec, unique, unique2);

        printf("new size: %d\n", newSize);
        auto* compressedData = new unsigned char[newSize];

        HashMap<PatternType, MarkerType> markerMap;
        HashMap<PatternType, uint16_t> marker2Map;

        for (const auto& pair : vec)
        {
            if (unique > 0)
            {
                markerMap[pair.first] = markers[markers.size() - unique];
                unique--;
            }
            else if (unique2 > 0)
            {
                marker2Map[pair.first] = markers2[markers2.size() - unique2];
                unique2--;
            }
        }

        int compressedIndex = 0;
        int i = 0;
        while (i <= size - sizeof(PatternType))
        {
            PatternType tempPattern;
            std::memcpy(&tempPattern, data + i, sizeof(PatternType));
            auto it = markerMap.find(tempPattern);
            if (it != markerMap.end())
            {
                compressedData[compressedIndex++] = it->second;
                i += sizeof(PatternType);
            }
            else
            {
                auto it2 = marker2Map.find(tempPattern);
                if (it2 != marker2Map.end())
                {
                    compressedData[compressedIndex++] = static_cast<unsigned char>(it2->second >> 8);   // High byte
                    compressedData[compressedIndex++] = static_cast<unsigned char>(it2->second & 0xFF); // Low byte
                    i += sizeof(PatternType);
                }
                else
                {
                    compressedData[compressedIndex++] = data[i++];
                }
            }
        }

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

        SortPatterns(topPatterns, patternMap, 1024);

        FilterPatterns(topPatterns);
        printf("Size %d\n", topPatterns.size());

        for (const auto& pair : topPatterns)
        {
            printf("Pattern: %04llx, Count: %d\n", pair.first, pair.second);
        }

        return GenerateCompressedData((BytePointer)data, size, topPatterns);
    }

    DataPointer<const char> DeCompress(const char* data, int size) { return {nullptr, 0}; }

} // namespace magique