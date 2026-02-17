// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <cstring>
#include <raylib/raylib.h>
#include <glaze/base64/base64.hpp>

#include <magique/util/Strings.h>
#include <magique/util/Logging.h>
#include <magique/util/Math.h>

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

    constexpr static Point KEY_POSITIONS[4][10] = {
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

    static PointI GetCharIndices(const KeyLayout layout, const char c)
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

    static Point GetCharPosition(const KeyLayout layout, const char c)
    {
        const auto [x, y] = GetCharIndices(layout, static_cast<char>(std::tolower(c)));
        if (x == UINT8_MAX)
            return {-1, -1};
        return KEY_POSITIONS[y][x]; // Switched
    }

    float StringDistancePhysical(const std::string_view s1, const std::string_view s2, const KeyLayout layout)
    {
        float distance = 0.0F;
        size_t i = 0;
        const float maxDistance = std::sqrt(9.5F * 9.5F + 3.0F * 3.0F);

        while (i < s1.size() && i < s2.size())
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

    bool StringIsSimilar(std::string_view original, std::string_view search, float tolerance)
    {
        if (original.empty())
        {
            return true;
        }

        const auto compareFunc = [](char ch1, char ch2)
        {
            return std::tolower(static_cast<unsigned char>(ch1)) == std::tolower(static_cast<unsigned char>(ch2));
        };
        const auto it = std::ranges::search(original, search, compareFunc);

        if (!it.empty())
        {
            return true;
        }
        if (StringDistancePhysical(original, search) > tolerance)
        {
            return true;
        }
        return false;
    }

    int StringReplace(char* buffer, const int bufferSize, const char* keyword, const char* replacement)
    {
        const auto lenKeyword = static_cast<int>(strlen(keyword));
        const auto lenReplacement = static_cast<int>(strlen(replacement));
        int replacements = 0;

        if (lenKeyword == 0)
        {
            return 0;
        }

        char* pos = strstr(buffer, keyword);
        while (pos != nullptr)
        {
            const auto lenBefore = static_cast<int>(pos - buffer);
            const auto lenAfter = static_cast<int>(strlen(pos + lenKeyword));

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

    int StringSetNewlines(char* buffer, const int bufferSize, const float width, const Font& font, const float fontSize)
    {
        if ((buffer == nullptr) || bufferSize == 0 || width <= 0)
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

                if (currentLineWidth > width && wordStart != buffer && *(wordStart - 1) != '\n')
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

    std::vector<std::string> StringSplit(std::string_view s, char delim)
    {
        std::vector<std::string> result;
        result.reserve(32);
        size_t start = 0;
        size_t end = 0;
        while ((end = s.find(delim, start)) != std::string_view::npos)
        {
            if (end != start)
            {
                result.emplace_back(s.substr(start, end - start));
            }
            start = end + 1;
        }
        if (start < s.size())
        {
            result.emplace_back(s.substr(start));
        }
        return result;
    }

    bool strcmpnc(const char* s1, const char* s2)
    {
        if ((s1 == nullptr) || (s2 == nullptr))
            return false;
        while ((*s1 != 0) && (*s2 != 0))
        {
            if (tolower(*s1) != tolower(*s2))
                return false;
            ++s1;
            ++s2;
        }
        return *s1 == *s2; // Ensure strings are of the same length
    }

    bool strncmpnc(const char* s1, const char* s2, int n)
    {
        if ((s1 == nullptr) || (s2 == nullptr) || n <= 0)
            return false;
        while (n-- > 0 && (*s1 != 0) && (*s2 != 0))
        {
            if (tolower(*s1) != tolower(*s2))
                return false;
            ++s1;
            ++s2;
        }
        return n < 0 || *s1 == *s2;
    }

    char* strstrnc(const char* haystack, const char* needle)
    {
        if (*needle == '\0')
        {
            return (char*)haystack;
        }

        for (; *haystack != '\0'; haystack++)
        {
            const char* h = haystack;
            const char* n = needle;
            while (*h != '\0' && *n != '\0' && tolower(*h) == tolower(*n))
            {
                h++;
                n++;
            }
            if (*n == '\0')
            {
                return (char*)haystack;
            }
        }

        return nullptr;
    }

    bool StringIsValidName(const char* text, int minLen, int maxLen)
    {
        if (text == nullptr)
        {
            return false;
        }
        int len = 0;
        while (*text != '\0')
        {
            if ((*text < 'A' || *text > 'Z') && (*text < 'a' || *text > 'z') && (*text < '0' || *text > '9'))
            {
                return false;
            }
            len++;
            text++;
        }
        return len >= minLen && len <= maxLen;
    }

    std::string StringToBase64(std::string_view input) { return glz::write_base64(input); }

    std::string StringFromBase64(std::string_view input) { return glz::read_base64(input); }

    const char* FormatFloat(float num, float cutoff, bool withSign)
    {
        if (withSign)
        {
            if (num < cutoff && !IsWholeNumber(num))
            {
                return TextFormat("%+.1f", num);
            }
            else
            {
                return TextFormat("%+d", (int)std::round(num));
            }
        }
        else
        {
            if (num < cutoff && !IsWholeNumber(num))
            {
                return TextFormat("%.1f", num);
            }
            else
            {
                return TextFormat("%d", (int)std::round(num));
            }
        }
    }

    const char* FormatGameplayNumber(float num)
    {
        if (num < 1e3)
        {
            return TextFormat("%.1f", num);
        }
        else if (num < 1e6)
        {
            return TextFormat("%.2fK", num / 1e3);
        }
        else if (num < 1e9)
        {
            return TextFormat("%.2fM", num / 1e6);
        }
        else
        {
            return TextFormat("%.2fB", num / 1e9);
        }
    }

    const char* FormatGameplayNumber(int64_t num)
    {
        if (num < 1000)
        {
            return TextFormat("%d", num);
        }
        return FormatGameplayNumber((float)num);
    }

    std::size_t StringHashFunc::operator()(const std::string& key) const { return StringHash(key.c_str()); }

    std::size_t StringHashFunc::operator()(const std::string_view& key) const
    {
        return StringHash(key.data(), key.size());
    }

    std::size_t StringHashFunc::operator()(const char* key) const { return StringHash(key); }

    bool StringEqualsFunc::operator()(const std::string& lhs, const std::string& rhs) const
    {
        return strcmp(lhs.c_str(), rhs.c_str()) == 0;
    }

    bool StringEqualsFunc::operator()(const std::string_view& lhs, const std::string& rhs) const
    {
        return lhs == std::string_view{rhs};
    }

    bool StringEqualsFunc::operator()(const char* lhs, const std::string& rhs) const
    {
        return strcmp(lhs, rhs.c_str()) == 0;
    }

} // namespace magique
