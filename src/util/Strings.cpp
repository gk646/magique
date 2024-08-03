#include <cmath>
#include <raylib/raylib.h>
#include <cxutil/cxstring.h>

#include <magique/util/Strings.h>

namespace magique
{
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
        // QWERTY layout [1-90a-z]
        {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0},  // 1 - 9 + 0
         {0, 2}, {4, 3}, {2, 3}, {2, 2}, {2, 1}, {3, 2}, {4, 2},                          // a - g
         {5, 2}, {7, 1}, {6, 2}, {7, 2}, {8, 2}, {6, 3}, {5, 3}, {8, 1}, {9, 1},          // h - p
         {0, 1}, {3, 1}, {1, 2}, {4, 1}, {6, 1}, {3, 3}, {1, 1}, {1, 3}, {5, 1}, {1, 3}}, // q - z
        // QWERTZ layout [1-90a-z]
        {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0},  // 1 - 9 + 0
         {0, 2}, {4, 3}, {2, 3}, {2, 2}, {2, 1}, {3, 2}, {4, 2},                          // a - g
         {5, 2}, {7, 1}, {6, 2}, {7, 2}, {8, 2}, {6, 3}, {5, 3}, {8, 1}, {9, 1},          // h - p
         {0, 1}, {3, 1}, {1, 2}, {4, 1}, {6, 1}, {3, 3}, {1, 1}, {1, 3}, {1, 3}, {5, 1}}, // q - z
    };

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
        const auto [x, y] = GetCharIndices(layout, static_cast<char>(std::tolower(c)));
        if (x == UINT8_MAX)
            return {-1, -1};
        return KEY_POSITIONS[y][x]; // Switched
    }

    int StringDistance(const char* s1, const char* s2, const bool caseSensitive)
    {
        return cxstructs::str_sort_levenshtein_case<16>(s1, s2, caseSensitive);
    }

    int StringDistance(const std::string& s1, const std::string& s2, const bool caseSensitive)
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
            return std::max(0.0F, 0.98F - 1.0F / 26.0F * diff);
        }
        return 1.0F - 1.0F / 26.0F * diff;
    }

    float StringSimilarity(const char* s1, const char* s2, const bool caseSensitive)
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

    float StringSimilarity(const std::string& s1, const std::string& s2, const bool caseSensitive)
    {
        return StringSimilarity(s1.c_str(), s2.c_str(), caseSensitive);
    }

    float StringDistancePhysical(const char* s1, const char* s2, KeyLayout layout)
    {
        float distance = 0.0F;
        int i = 0;
        const float maxDistance = std::sqrt(9.5F * 9.5F + 3.0F * 3.0F);

        while (s1[i] != '\0' && s2[i] != '\0')
        {
            const auto [x1, y1] = GetCharPosition(layout, s1[i]);
            const auto [x2, y2] = GetCharPosition(layout, s2[i]);
            i++;

            if (x1 < 0 || x2 < 0) // Invalid character
                continue;

            const auto dist = std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
            distance += dist;
        }

        // Normalize the distance
        const float normalizedDistance = distance / maxDistance / static_cast<float>(i);

        const float similarity = std::max(0.0F, 1.0F - normalizedDistance);

        return similarity;
    }

    float StringDistancePhysical(const std::string& s1, const std::string& s2, const KeyLayout layout)
    {
        return StringDistancePhysical(s1.c_str(), s2.c_str(), layout);
    }

    int ReplaceInBuffer(char* buffer, const int bufferSize, const char* keyword, const char* replacement)
    {
        const size_t lenKeyword = strlen(keyword);
        const size_t lenReplacement = strlen(replacement);
        int replacements = 0;

        if (lenKeyword == 0)
            return 0;

        char* pos = strstr(buffer, keyword);
        while (pos)
        {
            const size_t lenBefore = pos - buffer;
            const size_t lenAfter = strlen(pos + lenKeyword);

            if (lenBefore + lenAfter + lenReplacement >= bufferSize)
            {
                pos = strstr(pos + lenKeyword, keyword);
                continue;
            }

            if (lenReplacement != lenKeyword)
            {
                memmove(pos + lenReplacement, pos + lenKeyword, lenAfter + 1);
            }

            memcpy(pos, replacement, lenReplacement);
            replacements++;

            pos = strstr(pos + lenReplacement, keyword);
        }

        return replacements;
    }

    int InsertNewlines(char* buffer, const int bufferSize, const float width, const Font& font, const float fontSize)
    {
        if (!buffer || bufferSize == 0 || width <= 0)
            return 0;

        int lineBreaks = 0;
        char* cursor = buffer;    // Pointer to the current position in the buffer
        char* wordStart = buffer; // Start of the current word
        char* lineStart = buffer;

        while (*cursor != '\0')
        {
            char* nextCursor = cursor + 1;
            const bool atEndOfWord = *cursor == ' ' || *cursor == '\n' || *nextCursor == '\0';

            if (atEndOfWord)
            {
                char temp = *cursor;
                *cursor = '\0';
                const float currentLineWidth = MeasureTextEx(font, lineStart, fontSize, 1.0F).x;
                *cursor = temp;

                if (currentLineWidth >= width && wordStart != buffer && *(wordStart - 1) != '\n')
                {
                    *(wordStart - 1) = '\n';
                    lineStart = wordStart;
                    lineBreaks++;
                }
                wordStart = nextCursor;
            }
            cursor = nextCursor;
        }

        return lineBreaks;
    }

    std::vector<std::string> SplitString(const char* s, const char delim)
    {
        std::vector<std::string> result;
        result.reserve(5);

        const char* start = s;
        const char* end = s;

        while (*end != '\0')
        {
            if (*end == delim)
            {
                if (start != end)
                {
                    result.emplace_back(start, end);
                }
                start = end + 1;
            }
            end++;
        }

        if (start != end)
        {
            result.emplace_back(start);
        }

        return result;
    }

    std::vector<std::string> SplitString(const std::string& s, const char delim)
    {
        return SplitString(s.c_str(), delim);
    }

    std::string& TrimLeadingWhitespace(std::string& str)
    {
        const char* start = str.c_str();
        while (*start && std::isspace(static_cast<unsigned char>(*start)))
        {
            ++start;
        }
        str.erase(0, start - str.c_str());
        return str;
    }

    const char* GetTimeString(int totalSeconds)
    {
        const int days = totalSeconds / 86400;
        const int hours = totalSeconds % 86400 / 3600;
        const int minutes = totalSeconds % 3600 / 60;
        const int seconds = totalSeconds % 60;
        return TextFormat("%id:%ih:%im:%is", days, hours, minutes, seconds);
    }

    uint32_t HashString(char const* s, const int salt) noexcept
    {
        uint32_t hash = 2166136261U + salt;
        while (*s != 0)
        {
            hash ^= static_cast<uint32_t>(*s++);
            hash *= 16777619U;
        }
        return hash;
    }


} // namespace magique