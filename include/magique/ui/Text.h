#ifndef MAGIQUE_TEXT_H
#define MAGIQUE_TEXT_H

#include <magique/core/Types.h>


// Text


// This module allows rendering text with formatted placeholders replaced by specified values.
// Per default uses CMake syntax: "Hello, ${PLAYER_NAME}!" -> "Hello, Jespar!"
// Uses raylibs text buffers internally.
// Thus returned strings follow the same rules as its TextFormat() (invalidated on further calls)
//

namespace magique
{
    // Sets the maximum of supported formats per individual calls
    // Default: 5
    void SetMaxFormats(int n);

    // Sets the prefix and suffix to search for when looking for formats
    // Default: ${ ... }
    void SetFormatMask(const char* prefix, const char* suffix);

    // Sets the value with which the specified format will be replaced
    // Note: If a given format already exists it will be overwritten silently!
    void SetFormatValue(const char* format, const char* val);
    void SetFormatValue(const char* format, float val);
    void SetFormatValue(const char* format, int val);

    // Draws text formatted with
    void DrawTextFmt(const char* text, const Font& font, float size);

    // Returns the formatted text with the current values
    // IMPORTANT: Becomes invalid after calling any other function that uses raylibs formatting
    const char* GetTextFmt(const char* text);

} // namespace magique

#endif //MAGIQUE_TEXT_H