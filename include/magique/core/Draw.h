// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_DRAW_H
#define MAGIQUE_DRAW_H

#include <raylib/raylib.h>
#include <magique/core/Types.h>

//===============================================
// Draw Module
//===============================================
// ................................................................................
// Here are all the magique drawing function that take its custom types
// Generally you cant really use raylib types anymore as texture are stored into atlases
// Rotation is always a value in degree clockwise starting from the top with 0
// Note: Take a look at the raylib cheatsheet for all draw functions: https://www.raylib.com/cheatsheet/cheatsheet.html
// .............................................................

namespace magique
{
    //================= TEXTURES =================//

    // Draws a given texture region - rotation happens around the middle point
    void DrawRegion(TextureRegion region, const Point& pos, bool flipX = false, Color tint = WHITE);

    void DrawRegionCentered(const TextureRegion& region, const Point& pos, Color tint = WHITE);

    // Draws the given texture region and scales it according to the destination rect
    // Note: Pass a negative width/height to mirror the texture horizontally/vertically
    // Rotation happens around the anchor (relative to the position)
    void DrawRegionPro(TextureRegion region, Rectangle dest, float rotation = 0, Point anchor = {}, Color tint = WHITE);

    // Draws the given frame from the sprite sheet
    void DrawSprite(SpriteSheet sheet, float x, float y, int frame, bool flipX = false, Color tint = WHITE);

    // Draws and scales the given frame of the sprite sheet into the destination rect
    void DrawSpriteEx(SpriteSheet sheet, Rectangle dest, int frame, float rotation, Color tint = WHITE);

    // Automatically culled with the current camera!
    // If you need to scale your textures, provide a scaling factor when you load the sheet
    // 'layer' is 0-based indexing and starts from the bottom up in draw order
    void DrawTileMap(const TileMap& tileMap, const TileSheet& tileSheet, int layer);

    //================= TEXT =================//

    // Draws the given text centered around the given position
    // Drop in replacement for DrawTextEx()
    void DrawTextCentered(const Font& f, const char* txt, Vector2 pos, float fs, float spc = 1.0F, Color tint = WHITE);

    // Draws the given text aligned so that it ends on the given x positions
    // Drop in replacement for DrawTextEx()
    void DrawTextRightBound(const Font& f, const char* txt, Vector2 pos, float fs, float spc = 1.0F, Color tint = WHITE);

    // Returns the amount of characters drawn until the given width is reached (not overstepped)
    int DrawTextUpTo(const Font& font, const char* text, Vector2 pos, float fontSize, float width, Color tint = WHITE);

    // Draws pixel perfect text - fsm is font size multiplier of the base font size
    void DrawPixelText(const Font& f, const char* txt, Vector2 pos, int fsm = 1, Color tint = WHITE);
    void DrawPixelTextCentered(const Font& f, const char* txt, Vector2 pos, int fsm = 1, Color tint = WHITE);
    void DrawPixelTextRightBound(const Font& f, const char* txt, Vector2 pos, int fsm = 1, Color tint = WHITE);

    // Highlights all numbers in a different color
    void DrawPixelTextNumbers(const Font& f, const char* txt, Vector2 pos, int fsm = 1, Color text = WHITE,
                              Color numbers = GRAY);

    // Draws a rectangle over the text block defined by "from" and "to" so it looks like its highlighted
    void DrawTextHighlight(int from, int to, const Font& f, const char* txt, Vector2 pos, float fs, float spc = 1.0F,
                           Color highlight = ColorAlpha(BLUE, 0.4));

    //================= SHAPES =================//

    // Draws a capsule given its top left coordinates and the radius of the two circles
    void DrawCapsule2D(float x, float y, float radius, float height, Color tint);

    // Draws the outlines of a capsule given by the top left coordinates the height and the radius of the two circles
    void DrawCapsule2DLines(float x, float y, float radius, float height, Color tint);

    // Draws the outlines of the given rectangle rotated
    void DrawRectangleLinesRot(const Rectangle& rect, float rotation, float pivotX, float pivotY, Color color);

    // Draws a triangle - points must be provided in counter-clockwise order
    void DrawTriangleRot(Vector2 p1, Vector2 p2, Vector2 p3, float rot, float pivotX, float pivotY, Color color);

    // Draw a triangle using lines - points must be provided in counter-clockwise order
    void DrawTriangleLinesRot(Vector2 p1, Vector2 p2, Vector2 p3, float rot, float pivotX, float pivotY, Color color);

} // namespace magique

#endif //MAGIQUE_DRAW_H
