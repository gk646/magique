#ifndef MAGIQUE_STRINGS_H
#define MAGIQUE_STRINGS_H

#include <string>
#include <magique/core/Types.h>

//-----------------------------------------------
// Strings Module
//-----------------------------------------------

// .....................................................................
namespace magique
{
    //----------------- SORTING -----------------//

    // Returns the levenshtein distance of the two strings
    // Only compares up to a length of 16 characters
    int WordDistance(const char* s1, const char* s2, bool caseSensitive = false);
    int WordDistance(const std::string& s1, const std::string& s2, bool caseSensitive = false);

    // Returns the similarity (0.0 - 1.0) of the two strings based on alphabetical distance
    // Only checks until either one ends and doesn't penalize unequal length -> bla and blabbbbbb = 1.0F
    float WordSimilarity(const char* s1, const char* s2, bool caseSensitive = false);
    float WordSimilarity(const std::string& s1, const std::string& s2, bool caseSensitive = false);

    // Returns the similarity (0.0 - 1.0) of the two strings based on their distance on the keyboard
    // This is useful for user input as it handles common mistakes better - Not case sensitive per design
    float KeyboardDistance(const char* s1, const char* s2, KeyLayout layout = KeyLayout::QWERTY);
    float KeyboardDistance(const std::string& s1, const std::string& s2, KeyLayout layout = KeyLayout::QWERTY);

    //----------------- OPERATIONS -----------------//

    const char* InsertNewLines();

    //----------------- FORMATTING -----------------//
    // IMPORTANT: All functions use raylibs TextFormat() and has same limitations -> only valid until called again

    // Returns the time string in d:h:m:s (day, hour, minute, second) based on passed seconds
    const char* GetTimeString(int seconds);


} // namespace magique

#endif //MAGIQUE_STRINGS_H