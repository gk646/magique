// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_RAYUTILS_H
#define MAGIQUE_RAYUTILS_H

#include <magique/core/Types.h>

//===============================================
// raylib Utils
//===============================================
// .....................................................................
// These are useful methods expanding raylib ones
// .....................................................................

namespace magique
{
    //================= UTIL =================//

    // Same as raylib's but returns a magique::Point
    Point GetMousePos();

    // Returns the screen dimension in a point
    Point GetScreenDims();

    // Loads a texture from memory (by first loading it as image)
    Texture LoadTextureFromMemory(const unsigned char* data, int size, const char* fileType = ".png");

    Point GetGamePadLeftStick(int gamepad, float deadZone = 0.2F);
    Point GetGamePadRightStick(int gamepad, float deadZone = 0.2F);

    // Activates the shader in the constructor and ends the shader in the destructor
    struct ShaderWrapper
    {
        ShaderWrapper(const Shader& shader);
        ~ShaderWrapper();
    };

    // Activates the render texture in the constructor and ends it in the destructor
    struct RenderTextureWrapper final
    {
        RenderTextureWrapper(const RenderTexture& texture);
        ~RenderTextureWrapper();
    };

    //================= DRAWING =================//

    // Draws a scaled render texture at the given position (automatically flips it)
    void DrawRenderTexture(const RenderTexture& texture, Point pos, Point scale = 1, Color tint = WHITE);
    void DrawRenderTextureEx(const RenderTexture& texture, Rect dest,float rotation = 0, Color tint = WHITE);

    // Returns the length of the text up to the specified index
    float MeasureTextUpTo(const char* text, int index, const Font& font, float fontSize, float spacing = 1.0F);

    // Returns how many characters can be drawn up to the given width (not overstepped)
    int CountTextUpTo(const char* text, float width, const Font& font, float fontSize, float spacing = 1.0F);

    // Measure text functions for pixel fonts - mult is the multiple of the base font size
    float MeasurePixelText(const char* text, const Font& font, int mult = 1);

    // Returns the roundness for DrawRectangleRounded* such that regardless of size has the same corner radius
    float GetRoundness(float radius, const Rectangle& bounds);

    // Draws a nice looking (rounded) pixel art outline with the given radius
    // First the outline (dark) then the actual line (lighter) and the filler on the edges (lightest)
    void DrawPixelOutline(const Rectangle& bounds, const Color& outline, const Color& border, const Color& filler,
                          float radius = 2);

    // Draws an unrounded pixel art outline
    void DrawPixelBorder(const Rect& bounds, const Color& outline, const Color& border);

    // Returns true if mouse is inside rect or on rect
    bool CheckCollisionMouseRect(const Rectangle& bounds);

    // Draws a horizontally centered texture
    void DrawTextureCenteredV(const Texture& texture, const Vector2& pos, const Color& tint);

    // Draws a partially filled rectangle with the given maximum bounds
    // Either fills up in the given direction
    void DrawFilledRect(const Rectangle& bounds, float fillPercent, Direction dir, Color tint = WHITE);

    // Draws text horizontally & vertically centered inside the rectangle
    void DrawTextCenteredRect(const Font& fnt, std::string_view txt, float fs, const Rectangle& bounds,
                              float spacing = 1.0F, Color tint = WHITE);

    // Draws a 2d rectangle with a shade at the bottom
    void DrawRectangleShaded(const Rectangle& bounds, const Color& tint, const Color& shade, float shadeMult = 0.1F);

    // Draws a frame - leaves out the edges for a slightly rounded look
    void DrawRectFrame(const Rectangle& bounds, const Color& tint);

    // Draws a frame but filled
    void DrawRectFrameFilled(const Rect& bounds, const Color& fill, const Color& outline);

    // Returns the normalized position on the screen of the given world pos (e.g. 0.0 - 1.0)
    // Uses GetWorldToScreen2D() with the engine camera
    // Note: This is necessary when passing the position to shaders
    Point GetWorldToScreen2DNorm(Point world, Point screen = GetScreenDims());

    // Scales and draws the given render texture at the correct position to fill most of the given dimensions
    // Scales only with multiples and in a way such that both x and y dimension must fit within the dimensions
    // Also correctly sets the mouse offset and scale such that the top left is {0,0}
    void DrawTruePixelartScale(RenderTexture texture);

    // Draws an arrow in the given bounds
    // Note: For optimal results width should be even and height should be more than width
    void DrawArrow(const Rect& bounds, Color tint = WHITE);

    struct MouseDragger final
    {
        // Updates the zoom and returns the new camera position based on mouse dragg when called
        Point update(Camera2D& camera, float zoomMult = 2, float min = 1, float max = 5);

        // Returns the offset from drag start to the current position
        Point getDragOffset(Camera2D& camera) const;

        // Returns the camera position when the drag was started
        Point getCameraDragStart() const;

    private:
        void resetDragPos(Camera2D& camera);
        Point dragStartScreen{};
        Point dragStartCamera{};
    };

} // namespace magique

#endif // MAGIQUE_RAYUTILS_H
