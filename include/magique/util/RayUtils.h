// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_RAYUTILS_H
#define MAGIQUE_RAYUTILS_H

#include <magique/fwd.hpp>

//===============================================
// raylib Utils
//===============================================
// .....................................................................
// These are useful methods expanding raylib ones
// .....................................................................

namespace magique
{
    // Returns the length of the text up to the specified index
    float MeasureTextUpTo(const char* text, int index, const Font& font, float fontSize, float spacing = 1.0F);

    // Returns a random float using raylib's GetRandomValue() - min and max included
    float GetRandomFloat(float min, float max);

    // Returns the top left point to center a rectangle (width and height) within the rectangle 'within'
    Vector2 GetCenteredPos(const Rectangle& within, float width, float height);

    // Returns the center point of the given rect
    Vector2 GetRectCenter(const Rectangle& rect);


} // namespace magique

#endif //MAGIQUE_RAYUTILS_H