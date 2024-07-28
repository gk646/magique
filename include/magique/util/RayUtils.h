#ifndef MAGIQUE_RAYUTILS_H
#define MAGIQUE_RAYUTILS_H

#include <raylib/raylib.h>

//-----------------------------------------------
// raylib Utils
//-----------------------------------------------
// .....................................................................
// These are useful methods expanding raylib ones
// .....................................................................

namespace magique
{

    // Returns the length of the text up to the specified index
    float MeasureTextUpTo(char* text, int index, const Font& font, float fontSize, float spacing = 1.0F);


} // namespace magique

#endif //MAGIQUE_RAYUTILS_H