#ifndef MAGIQUE_TEXT_H
#define MAGIQUE_TEXT_H

#include <raylib/raylib.h>

//-----------------------------------------------
// Text Format Module
//-----------------------------------------------
// .....................................................................
// This module allows rendering text with placeholders replaced by specified values.
// To denote a placeholder you have to use the saved placeholder string and the
// Per default uses CMake syntax: "Hello, ${PLAYER_NAME}!" -> "Hello, Jespar!"
// Uses raylibs text buffers internally.
// Thus returned strings follow the same rules as its TextFormat() (invalidated on further calls)
// Supports up to 255 values for each type - Maximum length of the formatted string is 512
// Note: This is implemented as efficiently as possible -> its quite performant
// .....................................................................

namespace magique
{
    //----------------- VALUES -----------------//

    // Sets the value with which the specified format will be replaced
    // Note: If a given format already exists it will be overwritten silently!
    // Maximum length for a string value is 32
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
    // IMPORTANT: returned string becomes invalid after calling any other function that uses raylibs formatting
    const char* GetFormattedText(const char* text);

    //----------------- CUSTOMIZE -----------------//

    // Sets the prefix to search for when looking for placeholders
    // Default: '$'
    void SetFormatPrefix(char prefix);

} // namespace magique

#endif //MAGIQUE_TEXT_H