// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STRINGS_H
#define MAGIQUE_STRINGS_H

#include <string>
#include <vector>
#include <magique/core/Types.h>

//===============================================
// Strings Module
//===============================================
// .....................................................................
// This module allows for advanced and useful string operations in the context of games.
// Check raylib for some more fundamental text options like number parsing etc.
// .....................................................................

namespace magique
{
    //================= SORTING =================//

    // Returns the similarity (0.0 - 1.0) of the two strings based on their distance on the keyboard
    // This is useful for user input as it handles common mistakes better - not case-sensitive per design
    // Only checks until either one ends and doesn't penalize unequal length -> bla and blabbbbbb = 1.0F
    float StringDistancePhysical(std::string_view s1, std::string_view s2, KeyLayout layout = KeyLayout::QWERTY);

    // Returns true if any condition is true:
    //  - original contains search as a substring anywhere (not case sensitive)
    //  - StringDistancePhysical > tolerance
    //  - search is empty
    // This is very useful for finding the best matching entry to a user input (list of items, spells...)
    // In this case original would be the item name, and compare the search string
    bool TextIsSimilar(std::string_view original, std::string_view search, float tolerance = 0.9);

    //================= OPERATIONS =================//

    // Replaces all occurrences of the given keyword inside buffer with the replacement string in-place
    // Returns the total amount of replacements
    int ReplaceInBuffer(char* buffer, int bufferSize, const char* keyword, const char* replacement);

    // Replaces spaces with newlines such that each line is drawn within the specified width in-place
    // Note: Does NOT change the size of the buffer
    // Returns: the number of linebreak inserted
    int InsertNewlines(char* buffer, int bufferSize, float width, const Font& font, float fontSize);

    // Returns a vector of string containing the chunks by splitting the string by delim
    // This is useful if you need to work with the strings and modify them a lot
    std::vector<std::string> SplitString(std::string_view s, char delim);

    // Removes any leading whitespace in-place and returns the string
    std::string& TrimLeadingWhitespace(std::string& s);

    // Returns true if the given strings match regardless of case
    // string-compare-no-case
    bool strcmpnc(const char* s1, const char* s2);

    // Returns true if the given strings match up to n characters regardless of case
    // string-compare-n-no-case
    bool strncmpnc(const char* s1, const char* s2, int n);

    // Case insensitive version of strstr
    // https://en.cppreference.com/w/c/string/byte/strstr
    char* strstrnc(const char* haystack, const char* needle);

    // Returns true if the given string conforms to:
    //  - At least "minLen" but not longer than "maxLen"
    //  - Characters are only:
    //      - Letters a-zA-Z
    //      - Numbers 0-9
    // That means no symbols like "%!?-_" or spaces
    bool TextIsPlayerName(const char* text, int minLen = 3, int maxLen = 16);

    //================= ENCODING =================//

    // Returns the size of the base64 representation of the bytes - useful to allocate an array that can hold it
    int GetBase64EncodedLength(int bytes);

    // Encodes the given string 's' into base64 representation
    //      - inputLen : length of the binary portion
    //      - outputLen: valid length of the output pointer
    // Note: Allows inplace modification of the input string (e.g. input and output being the same array)
    void EncodeBase64(const char* input, int inputLen, char* output, int outputLen);

    // Returns the base64 encoded input as string
    std::string EncodeBase64(std::string input);

    // Decodes the given base64 input inplace
    void DecodeBase64(char* input);

    // Returns the decoded base64 input as string - makes a single copy
    std::string DecodeBase64(std::string input);

    //================= FORMATTING =================//

    // Returns the time string in d:h:m:s (day, hour, minute, second) based on passed seconds
    const char* GetTimeString(int seconds);

    // Formats a float number so its nicely readable
    // If it's lower than cutoff OR a whole number (e.g. 3.0) its formatted as integer, else with 1 decimal as float
    const char* FormatFloat(float num, float cutoff = 100.0F, bool withSign = false);

    // Formats the given number for displaying it in gameplay context:
    // Shortens to at most 5 digits: 3 before dot, 2 after
    // Uses K/M/B to shorten thousands, millions and billions
    // e.g. 2.4 / 32.45K / 332.53K / 233.41M / 33.52B
    const char* FormatGameplayNumber(float num);
    const char* FormatGameplayNumber(int64_t num);

    //================= HASHING =================//

    // Uses fnav32a1 to hash the string - aimed to be fast not secure!
    constexpr uint32_t HashString(char const* s) noexcept;
    constexpr uint32_t HashString(const char* s, size_t len) noexcept;

    // Useful for passing to a hashmap for to enable transparent lookups (avoids conversion)
    struct StringHashFunc
    {
        using is_transparent = void;
        std::size_t operator()(const std::string& key) const;
        std::size_t operator()(const std::string_view& key) const;
        std::size_t operator()(const char* key) const;
    };

    // Useful for passing to a hashmap for to enable transparent lookups (avoids conversion)
    struct StringEqualsFunc
    {
        using is_transparent = void;
        bool operator()(const std::string& lhs, const std::string& rhs) const;
        bool operator()(const std::string_view& lhs, const std::string& rhs) const;
        bool operator()(const char* lhs, const std::string& rhs) const;
    };

} // namespace magique


//================= IMPLEMENTATION =================//


namespace magique
{
    constexpr uint32_t HashString(char const* s) noexcept
    {
        uint32_t hash = 2166136261U;
        while (*s != 0)
        {
            hash ^= static_cast<uint32_t>(*s++);
            hash *= 16777619U;
        }
        return hash;
    }

    constexpr uint32_t HashString(const char* s, const size_t len) noexcept
    {
        uint32_t hash = 2166136261U;
        size_t counter = 0;
        while (counter < len)
        {
            hash ^= static_cast<uint32_t>(s[counter++]);
            hash *= 16777619U;
        }
        return hash;
    }

} // namespace magique

#endif //MAGIQUE_STRINGS_H
