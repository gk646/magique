// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <raylib/raylib.h>
#include <cxutil/cxstring.h>

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

    int StringDistance(const char* s1, const char* s2, const bool caseSensitive)
    {
        return cxstructs::str_sort_levenshtein_case<16>(s1, s2, caseSensitive);
    }

    int StringDistance(const std::string& s1, const std::string& s2, const bool caseSensitive)
    {
        return cxstructs::str_sort_levenshtein_case<16>(s1.c_str(), s2.c_str(), caseSensitive);
    }

    float StringDistancePhysical(const char* s1, const char* s2, const KeyLayout layout)
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

    bool TextIsSimilar(const char* original, const char* search, float tolerance)
    {
        if (*original == '\0')
        {
            return true;
        }
        if (strstrnc(original, search) != nullptr)
        {
            return true;
        }
        if (StringDistancePhysical(original, search) > tolerance)
        {
            return true;
        }
        return false;
    }

    int ReplaceInBuffer(char* buffer, const int bufferSize, const char* keyword, const char* replacement)
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

    int InsertNewlines(char* buffer, const int bufferSize, const float width, const Font& font, const float fontSize)
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

    std::string& TrimLeadingWhitespace(std::string& s)
    {
        const char* start = s.c_str();
        while (*start && std::isspace(static_cast<unsigned char>(*start)))
        {
            ++start;
        }
        s.erase(0, start - s.c_str());
        return s;
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

    bool TextIsPlayerName(const char* text, int minLen, int maxLen)
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

    int GetBase64EncodedLength(const int bytes) { return 4 * ((bytes + 2) / 3); }

    constexpr char BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz"
                                    "0123456789+/";

    void EncodeBase64(const char* input, const int inputLen, char* output, const int outputLen)
    {
        const int base64Len = GetBase64EncodedLength(inputLen);
        if (outputLen < base64Len + 1)
        {
            LOG_WARNING("The output buffer isn't large enough to contain the encoded string");
            return;
        }

        int i = inputLen - 1;
        int o = base64Len - 1;

        output[base64Len] = '\0';

        const int mod = inputLen % 3;
        if (mod == 1)
        {
            const uint32_t value = static_cast<uint8_t>(input[i--]) << 16;
            output[o--] = '=';
            output[o--] = '=';
            output[o--] = BASE64_CHARS[(value >> 12) & 0x3F];
            output[o--] = BASE64_CHARS[(value >> 18) & 0x3F];
        }
        else if (mod == 2)
        {
            uint32_t value = (static_cast<uint8_t>(input[i - 1]) << 8) | static_cast<uint8_t>(input[i]);
            value <<= 8;
            output[o--] = '=';
            output[o--] = BASE64_CHARS[(value >> 6) & 0x3F];
            output[o--] = BASE64_CHARS[(value >> 12) & 0x3F];
            output[o--] = BASE64_CHARS[(value >> 18) & 0x3F];
            i -= 2;
        }

        while (i >= 2)
        {
            const uint32_t value = (static_cast<uint8_t>(input[i - 2]) << 16) |
                (static_cast<uint8_t>(input[i - 1]) << 8) | static_cast<uint8_t>(input[i]);

            output[o--] = BASE64_CHARS[value & 0x3F];
            output[o--] = BASE64_CHARS[(value >> 6) & 0x3F];
            output[o--] = BASE64_CHARS[(value >> 12) & 0x3F];
            output[o--] = BASE64_CHARS[(value >> 18) & 0x3F];

            i -= 3;
        }
    }

    std::string EncodeBase64(std::string input)
    {
        const int binarySize = static_cast<int>(input.size());
        const int base64Len = GetBase64EncodedLength(static_cast<int>(input.size())) + 1;
        input.resize(base64Len);
        EncodeBase64(input.c_str(), binarySize, input.data(), base64Len);
        input.pop_back(); // pop the '\0' terminator
        return input;
    }

    static int Base64CharToValue(const char c)
    {
        if (c >= 'A' && c <= 'Z')
            return c - 'A';
        if (c >= 'a' && c <= 'z')
            return c - 'a' + 26;
        if (c >= '0' && c <= '9')
            return c - '0' + 52;
        if (c == '+')
            return 62;
        if (c == '/')
            return 63;
        return -1;
    }

    void DecodeBase64(char* input)
    {
        const int inputLen = static_cast<int>(strlen(input));
        int i = 0;
        int b = 0;
        while (b < inputLen)
        {
            const auto b1 = Base64CharToValue(input[b]);
            const auto b2 = Base64CharToValue(input[b + 1]);
            const auto b3 = Base64CharToValue(input[b + 2]);
            const auto b4 = Base64CharToValue(input[b + 3]);

            if (b1 == -1 || b2 == -1 || (input[b + 2] != '=' && b3 == -1) || (input[b + 3] != '=' && b4 == -1))
            {
                LOG_WARNING("Invalid base64 character detected");
                return;
            }

            const uint32_t value = (b1 << 18) | (b2 << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);

            if (input[b + 3] == '=') [[unlikely]] // padding
            {
                if (input[b + 2] == '=') // `==` case, 1 byte of data
                {
                    input[i++] = static_cast<char>(value >> 16);
                }
                else // `=` case, 2 bytes of data
                {
                    input[i++] = static_cast<char>(value >> 16);
                    input[i++] = static_cast<char>(value >> 8);
                }
                break;
            }

            input[i] = static_cast<char>(value >> 16);
            input[i + 1] = static_cast<char>(value >> 8);
            input[i + 2] = static_cast<char>(value);

            i += 3;
            b += 4;
        }
        input[i] = '\0';
    }

    std::string DecodeBase64(std::string input)
    {
        DecodeBase64(input.data());
        int size = static_cast<int>(input.size()) - 1;
        while (size > 0)
        {
            if (input[size] == '\0')
            {
                break;
            }
            --size;
        }
        if (size > 0)
        {
            input.resize(size);
        }
        return input;
    }

    const char* GetTimeString(const int totalSeconds)
    {
        const int days = totalSeconds / 86400;
        const int hours = totalSeconds % 86400 / 3600;
        const int minutes = totalSeconds % 3600 / 60;
        const int seconds = totalSeconds % 60;
        return TextFormat("%id:%ih:%im:%is", days, hours, minutes, seconds);
    }

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

} // namespace magique
