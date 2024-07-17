#ifndef MAGIQUE_STRINGS_H
#define MAGIQUE_STRINGS_H

#include <string>
#include <magique/core/Types.h>

namespace magique
{

    //----------------- SORTING -----------------//

    // Returns the levenshtein distance of the two strings
    int WordDistance(const char* s1, const char* s2, bool caseSensitive = false);

    // Returns the levenshtein distance of the two strings
    int WordDistance(const std::string& s1, const std::string& s2, bool caseSensitive = false);

    // Returns the similarity (0.0 - 1.0) of the two strings are based on alphabetical distance
    float WordSimilarity(const char* s1, const char* s2, bool caseSensitive = false);

    // Returns the similarity (0.0 - 1.0) of the two strings are based on alphabetical distance
    float WordSimilarity(const std::string& s1, const std::string& s2, bool caseSensitive = false);

    // Returns the similarity (0.0 - 1.0) of the two strings based on their distance on the keyboard
    float WordSimilarityKeyBoard(const char* s1, const char* s2, KeyboardLayout layout);

    // Returns the similarity (0.0 - 1.0) of the two strings based on their distance on the keyboard
    float WordSimilarityKeyBoard(const std::string& s1, const std::string& s2, KeyboardLayout layout);

} // namespace magique

#endif //MAGIQUE_STRINGS_H