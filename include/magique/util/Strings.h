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
    float WordSimilarity(const char* s1, const char* s2, bool caseSensitive = false);
    float WordSimilarity(const std::string& s1, const std::string& s2, bool caseSensitive = false);

    // Returns the similarity (0.0 - 1.0) of the two strings based on their distance on the keyboard
    // This is useful for user input as it handles common mistakes better
    float WordSimilarityKeyBoard(const char* s1, const char* s2, KeyboardLayout layout);
    float WordSimilarityKeyBoard(const std::string& s1, const std::string& s2, KeyboardLayout layout);


    //----------------- OPERATIONS -----------------//

    const char* InsertNewLines();


    //----------------- FORMATTING -----------------//
    // IMPORTANT: Uses raylibs TextFormat() and has same limitations -> only valid until called again

    // Returns the time string in d:h:m:s (day, hour, minute, second) based on passed seconds
    const char* GetTimeString(int seconds);


} // namespace magique

#endif //MAGIQUE_STRINGS_H