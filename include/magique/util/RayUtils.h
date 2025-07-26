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

    // Returns how many characters can be drawn up to the given width (not overstepped)
    int CountTextUpTo(const char* text, float width, const Font& font, float fontSize, float spacing = 1.0F);

    // Measure text functions for pixel fonts - mult is the multiple of the base font size
    float MeasurePixelText(const char* text, const Font& font, int mult = 1);

    // Returns a random float using raylib's GetRandomValue() - min and max included
    float GetRandomFloat(float min, float max);

    // Returns the top left point to center a rectangle (width and height) within the rectangle 'within'
    Vector2 GetCenteredPos(const Rectangle& within, float width, float height);

    // Returns the center point of the given rect
    Vector2 GetRectCenter(const Rectangle& rect);

    // Returns a rect with the given dimensions centered on the given position
    Rectangle GetCenteredRect(const Point& center, float width, float height);

    // Toggles fullscreen more reliably
    // First maximizes the window then sets the monitors max size then toggles fullscreen
    void ToggleFullscreenEx();

} // namespace magique

#endif //MAGIQUE_RAYUTILS_H