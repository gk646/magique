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

    // Gets the enlarged rectangle - also adjusts x and y so it stays in the same relative spot
    // Note: Also works in negative direction
    Rectangle GetEnlargedRect(const Rectangle& rect, float width, float height);

    // Returns the roundness for DrawRectangleRounded* such that regardless of size has the same corner radius
    float GetRoundness(float radius, const Rectangle& bounds);

    // Toggles fullscreen more reliably
    // First maximizes the window then sets the monitors max size then toggles fullscreen
    void ToggleFullscreenEx();

    // Loads a texture from memory (by first loading it as image)
    Texture LoadTextureFromMemory(const unsigned char* data, int size, const char* fileType = ".png");

    // Draws the full texture at the specified position and scale
    void DrawTextureScaled(const Texture& texture, const Rectangle& dest, const Color& tint);

    // Draws a scaled render texture at the given position (automatically flips it)
    void DrawRenderTexture(const RenderTexture& texture, const Vector2& pos, float scale, const Color& tint);

    // Draws a nice looking (rounded) pixel art outline with the given radius
    // First the outline (dark) then the actual line (lighter) and the filler on the edges (lightest)
    void DrawPixelOutline(const Rectangle& bounds, const Color& outline, const Color& border, const Color& filler,
                          float radius = 2);

    // Draws an unrounded pixel art outline
    void DrawPixelBorder(const Rectangle& bounds, const Color& outline, const Color& border);

    // Returns true if mouse is inside rect or on rect
    bool CheckCollisionMouseRect(const Rectangle& bounds);

    // Draws a horizontally centered texture
    void DrawCenteredTextureV(const Texture& texture, const Vector2& pos, const Color& tint);

    // Draws a partially filled rectangle with the given maximum bounds
    // Either fills up in the given direction
    void DrawFilledRect(const Rectangle& bounds, float fillPercent, Direction dir, const Color& tint);

    void DrawCenteredTextRect(const Font& fnt, const char* txt, float fs, const Rectangle& bounds, float spacing,
                              const Color& tint);

    // Draws a 2d rectangle with a shade at the bottom
    void DrawRectangleShaded(const Rectangle& bounds, const Color& tint, const Color& shade, float shadeMult = 0.1F);

    // Draws a frame - leaves out the edges for a slightly rounded look
    void DrawRectFrame(const Rectangle& bounds, const Color& tint);

    // Draws a frame but filled
    void DrawRectFrameFilled(const Rectangle& bounds, const Color& fill, const Color& outline);

    // Scales and draws the given render texture at the correct position to fill most of the screen
    // Scales only with multiples and in a way such that both x and y dimension must fit within the screen
    // Also correctly sets the mouse offset and scale such that the top left is {0,0}
    void DrawTruePixelartScale(RenderTexture texture);

} // namespace magique

#endif //MAGIQUE_RAYUTILS_H