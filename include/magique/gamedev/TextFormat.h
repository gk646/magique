// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_TEXT_FORMAT_H
#define MAGIQUE_TEXT_FORMAT_H

#include <string>
#include <optional>
#include <raylib/raylib.h>

//===============================================
// Text Formatting Module
//===============================================
// .....................................................................
// This module allows formatting and rendering of text with dynamic placeholders.
// To denote a placeholder you have to use the saved placeholder string and the prefix + enclosing symbol
// Note: This is implemented as efficiently as possible -> its really performant
// Limitations:
//              - Maximum of 255 Values for each type -> 775 total (int, float, string)
//              - Maximum length of string values is 64
//              - Maximum length of any placeholder is 64
//
// Note: Per default uses CMake/Bash syntax: "Hello, ${PLAYER_NAME}!" -> "Hello, Jespar!"
// .....................................................................

namespace magique
{
    //================= VALUES =================//

    // Sets the value with which the specified placeholder will be replaced
    // Given string has to be valid until this method returns!
    // Note: If a given format already exists it will be overwritten silently!
    // Note: "placeholder" is only the value without prefix and braces e.g. PLAYER_NAME instead of ${PLAYER_NAME}
    void FormatSetValue(std::string_view placeholder, const std::string_view& val);
    void FormatSetValue(std::string_view placeholder, float val);
    void FormatSetValue(std::string_view placeholder, int val);

    // Returns a modifiable reference to the value of this placeholder
    // Note: Type has to be specified manually - int, float or std::string
    template <typename T>
    std::optional<std::reference_wrapper<T>> FormatGetValue(const std::string_view& placeholder);

    //================= FORMAT =================//

    // Formats and draws the given text with the current placeholder state
    // Note: This function tries to format the given text! If you already have a formatted text use DrawText...()
    void DrawTextFmt(const Font& font, const char* fmt, Vector2 pos, float size, float spacing = 1, Color color = WHITE);

    // Formats and returns the given text with the current placeholder state
    std::string_view FormatGetText(std::string_view text);

    //================= CUSTOMIZE =================//

    // Sets the prefix to search for when looking for placeholders
    // Default: '$'
    void FormatSetPrefix(char prefix = '$');

} // namespace magique

#endif // MAGIQUE_TEXT_FORMAT_H
