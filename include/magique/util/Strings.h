#ifndef MAGIQUE_STRINGS_H
#define MAGIQUE_STRINGS_H

#include <string>
#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Strings Module
//-----------------------------------------------
// .....................................................................
// This module allows for advanced and useful string operations in the context of games.
// Check raylib for some more fundamental text options like number parsing etc.
// .....................................................................

namespace magique
{
    //----------------- SORTING -----------------//

    // Returns the levenshtein distance of the two strings
    // Only compares up to a length of 16 characters
    int StringDistance(const char* s1, const char* s2, bool caseSensitive = false);
    int StringDistance(const std::string& s1, const std::string& s2, bool caseSensitive = false);

    // Returns the similarity (0.0 - 1.0) of the two strings based on their distance on the keyboard
    // This is useful for user input as it handles common mistakes better - not case-sensitive per design
    // Only checks until either one ends and doesn't penalize unequal length -> bla and blabbbbbb = 1.0F
    float StringDistancePhysical(const char* s1, const char* s2, KeyLayout layout = KeyLayout::QWERTY);
    float StringDistancePhysical(const std::string& s1, const std::string& s2, KeyLayout layout = KeyLayout::QWERTY);

    // Returns the similarity (0.0 - 1.0) of the two strings based on alphabetical distance
    // This is very useful for finding the best matching entry to a user input (list of items, spells...)
    // Only checks until either one ends and doesn't penalize unequal length -> bla and blabbbbbb = 1.0F
    float StringSimilarity(const char* s1, const char* s2, bool caseSensitive = false);
    float StringSimilarity(const std::string& s1, const std::string& s2, bool caseSensitive = false);

    //----------------- OPERATIONS -----------------//

    // Replaces all occurrences of the given keyword inside buffer with the replacement string in-place
    // Returns the total amount of replacements
    int ReplaceInBuffer(char* buffer, int bufferSize, const char* keyword, const char* replacement);

    // Inserts newlines so that each line is drawn within the specified width in-place
    // Returns the number of linebreak inserted
    int InsertNewlines(char* buffer, int bufferSize, float width, const Font& font, float fontSize);

    // Returns a vector of string containing the chunks by splitting the string by delim
    // This is useful if you need to work with the strings and modify them a lot
    std::vector<std::string> SplitString(const char* s, char delim);
    std::vector<std::string> SplitString(const std::string& s, char delim);

    // Removes any leading whitespace in-place and returns the string
    std::string& TrimLeadingWhitespace(std::string& s);

    //----------------- ENCODING -----------------//

    // Returns the size of the base64 representation of the bytes - useful to allocate an array that can hold it
    int GetBase64EncodedLength(int bytes);

    // Returns true if the given string is base64
    bool IsBase64(const char* s);

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

    //----------------- FORMATTING -----------------//
    // IMPORTANT: All functions in this section use raylibs TextFormat() and have the same limitations
    // -> only valid until called again

    // Returns the time string in d:h:m:s (day, hour, minute, second) based on passed seconds
    const char* GetTimeString(int seconds);

    //----------------- HASHING -----------------//

    // Uses fnav32a1 to hash the string
    constexpr uint32_t HashString(char const* s) noexcept;

} // namespace magique


//----------------- IMPLEMENTATION -----------------//


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

} // namespace magique

#endif //MAGIQUE_STRINGS_H