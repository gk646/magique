#ifndef MAGIQUE_TEXT_H
#define MAGIQUE_TEXT_H

#include <raylib/raylib.h>

//-----------------------------------------------
// Text Format Module
//-----------------------------------------------
// .....................................................................
// This module allows rendering text with dynamic placeholders replaced by specified values.
// To denote a placeholder you have to use the saved placeholder string and the
// Per default uses CMake syntax: "Hello, ${PLAYER_NAME}!" -> "Hello, Jespar!"
// Note: This is implemented as efficiently as possible -> its quite performant
// Limitations:
//              - Maximum of 255 Values for each type -> 885 total (int, float, string)
//              - Maximum length of string values is 64
//              - Maximum length of any placeholder is 64
// .....................................................................

namespace magique
{
    //----------------- VALUES -----------------//

    // Sets the value with which the specified placeholder will be replaced
    // Note: If a given format already exists it will be overwritten silently!
    void SetFormatValue(const char* placeholder, const char* val);
    void SetFormatValue(const char* placeholder, const std::string& val);
    void SetFormatValue(const char* placeholder, float val);
    void SetFormatValue(const char* placeholder, int val);

    // Returns a modifiable reference to the value of this placeholder
    // Note: Type has to be specified manually - int, float or std::string
    template <typename T>
    T& GetFormatValue(const char* placeholder);

    //----------------- FORMAT -----------------//

    // Formats and draws the given text with the current placeholder state
    // Note: Acts as a drop in replacement for DrawTextEx()
    void DrawTextFmt(const Font& font, const char* text, Vector2 pos, float size, float spacing, Color tint = WHITE);

    // Formats and returns the given text with the current placeholder state
    // IMPORTANT: returned string will only be valid until this method is called again
    const char* GetFormattedText(const char* text);

    //----------------- CUSTOMIZE -----------------//

    // Sets the prefix to search for when looking for placeholders
    // Default: '$'
    void SetFormatPrefix(char prefix);

} // namespace magique

#endif //MAGIQUE_TEXT_H