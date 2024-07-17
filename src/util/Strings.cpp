#include <magique/util/Strings.h>
#include <cmath>

#include <raylib/raylib.h>
#include <cxutil/cxstring.h>

namespace magique
{
    struct Point
    {
        float x;
        float y;
    };

    struct PointI
    {
        uint8_t x;
        uint8_t y;
    };
    // '1' to '0'
    // 'q' to 'p'
    // 'a' to 'l'
    // 'z' to 'm'
    constexpr Point KEY_POSITIONS[4][10] = {
        {
            {0, 0},
            {1, 0},
            {2, 0},
            {3, 0},
            {4, 0},
            {5, 0},
            {6, 0},
            {7, 0},
            {8, 0},
            {9, 0},
        },
        {{0.5, 1}, {1.5, 1}, {2.5, 1}, {3.5, 1}, {4.5, 1}, {5.5, 1}, {6.5, 1}, {7.5, 1}, {8.5, 1}, {9.5, 1}},
        {{0.75, 2}, {1.75, 2}, {2.75, 2}, {3.75, 2}, {4.75, 2}, {5.75, 2}, {6.75, 2}, {7.75, 2}, {8.75, 2}, {}},
        {{1.25, 3}, {2.25, 3}, {3.25, 3}, {4.25, 3}, {5.25, 3}, {6.25, 3}, {7.25, 3}, {}, {}, {}},
    };

    constexpr PointI CHAR_LOOKUP[2][36] = {
        // QWERTY layout
        {
            {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, // '1' to '0'
            {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, // 'q' to 'p'
            {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8},         // 'a' to 'l'
            {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}                          // 'z' to 'm'
        },
        // QWERTZ layout
        {
            {0, 0}, {0, 1}, {0, 2}, {0, 3}, {3, 0}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, // '1' to '0'
            {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, // 'q' to 'p'
            {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8},         // 'a' to 'l'
            {0, 4}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}                          // 'y' to 'm'
        }};

    PointI GetCharIndices(const KeyLayout layout, const char c)
    {
        const int num = static_cast<int>(layout);
        if (c >= '1' && c <= '9')
            return CHAR_LOOKUP[num][c - '1'];
        if (c == '0')
            return CHAR_LOOKUP[num][9];
        if (c >= 'a' && c <= 'z')
            return CHAR_LOOKUP[num][10 + (c - 'a')];
        return {UINT8_MAX, UINT8_MAX};
    }

    Point GetCharPosition(const KeyLayout layout, const char c)
    {
        const auto [x, y] = GetCharIndices(layout, c);
        if (x == UINT8_MAX)
            return {-1, -1};
        return KEY_POSITIONS[x][y];
    }


    int WordDistance(const char* s1, const char* s2, const bool caseSensitive)
    {
        return cxstructs::str_sort_levenshtein_case<16>(s1, s2, caseSensitive);
    }

    int WordDistance(const std::string& s1, const std::string& s2, const bool caseSensitive)
    {
        return cxstructs::str_sort_levenshtein_case<16>(s1.c_str(), s2.c_str(), caseSensitive);
    }

    float GetCharacterSimilarity(char base, char check, const bool caseSensitive) noexcept
    {
        if (base == check)
            return 1.0F;

        bool casePenalty = false;

        const bool baseIsUpper = base < 91;
        const bool checkIsUpper = check < 91;

        if (caseSensitive && baseIsUpper != checkIsUpper)
        {
            casePenalty = true;
        }

        base -= baseIsUpper ? 65 : 97;
        check -= checkIsUpper ? 65 : 97;

        auto diff = static_cast<float>(std::abs(base - check));
        diff = std::min(26.0F, diff);
        if (casePenalty) [[unlikely]]
        {
            return std::max(0.0F, 0.96F - 1.0F / 26.0F * diff);
        }
        return 1.0F - 1.0F / 26.0F * diff;
    }

    float WordSimilarity(const char* s1, const char* s2, bool caseSensitive)
    {
        if (s1 == nullptr || s2 == nullptr || s1[0] == '\0' || s2[0] == '\0')
            return 0.0F;

        float sum = 0.0F;
        int i = 0;

        // Iterate until either string ends
        while (s1[i] != '\0' && s2[i] != '\0')
        {
            sum += GetCharacterSimilarity(s1[i], s2[i], caseSensitive);
            i++;
        }

        return sum / static_cast<float>(i);
    }

    float WordSimilarity(const std::string& s1, const std::string& s2, const bool caseSensitive)
    {
        return WordSimilarity(s1.c_str(), s2.c_str(), caseSensitive);
    }

    float KeyboardDistance(const char* s1, const char* s2, KeyLayout layout)
    {
        float distance = 0.0F;
        int i = 0;
        const float maxDistance = std::sqrt(9.0F * 9.0F + 3.0F * 3.0F);

        while (s1[i] != '\0' && s2[i] != '\0')
        {
            const auto pos1 = GetCharPosition(layout, s1[i]);
            const auto pos2 = GetCharPosition(layout, s2[i]);
            i++;

            if (pos1.x < 0 || pos2.x < 0) // Invalid character
                continue;

            const auto dist = std::sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y));
            distance += dist;
        }

        // Normalize the distance
        float normalizedDistance = distance / maxDistance;
        // Calculate similarity
        float similarity = std::max(0.0F, 1.0F - normalizedDistance);

        return similarity;
    }

    float KeyboardDistance(const std::string& s1, const std::string& s2, const KeyLayout layout)
    {
        return KeyboardDistance(s1.c_str(), s2.c_str(), layout);
    }

    const char* GetTimeString(int totalSeconds)
    {
        const int days = totalSeconds / 86400;
        const int hours = totalSeconds % 86400 / 3600;
        const int minutes = totalSeconds % 3600 / 60;
        const int seconds = totalSeconds % 60;
        return TextFormat("%id:%ih:%im:%is", days, hours, minutes, seconds);
    }


} // namespace magique