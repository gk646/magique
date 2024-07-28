#ifndef MAGIQUE_DRAW_H
#define MAGIQUE_DRAW_H

#include <raylib/raylib.h>
#include <magique/core/Types.h>

//-----------------------------------------------
// Draw Module
//-----------------------------------------------
// ................................................................................
// Here are all the magique drawing function that take its custom types
// Generally you cant really use raylib types anymore as texture are stored into atlases
// ................................................................................

namespace magique
{
    //----------------- TEXTURES -----------------//

    // Draws a given texture region
    void DrawRegion(TextureRegion region, float x, float y, bool flipX = false, Color tint = WHITE);

    // Draws a given texture region
    // Rotation happens around the middle point
    void DrawRegionEx(TextureRegion region, float x, float y, float rotation, bool flipX = false, Color tint = WHITE);

    // Draws the given texture region and scales it according to the destination rect
    // Pass -width to flip the texture horizontally - rotation happens around the pivot point (rotX, rotY)
    void DrawRegionPro(TextureRegion region, Rectangle dest, float rotation, float rotX, float rotY, Color tint = WHITE);

    // Draws the given frame from the sprite sheet
    void DrawSprite(SpriteSheet sheet, float x, float y, int frame, bool flipX = false, Color tint = WHITE);

    // Draws and scales the given frame of the sprite sheet into the destination rect
    void DrawSpriteEx(SpriteSheet sheet, Rectangle dest, int frame, float rotation, Color tint = WHITE);

    // Automatically culled with the current camera!
    // If you need to scale your textures, provide a scaling factor when you load the sheet
    // 'layer' is 0-based indexing and starts from the bottom up in draw order
    void DrawTileMap(const TileMap& tileMap, const TileSheet& tileSheet, int layer);

    //----------------- TEXT -----------------//

    // Draws the given text centered around the given position
    // Drop in replacement for DrawTextEx()
    void DrawCenteredText(const Font& f, const char* txt, Vector2 pos, float fs, float spc = 1.0F, Color tint = WHITE);

    // Draws the given text aligned so that it ends on the given x positions
    // Drop in replacement for DrawTextEx()
    void DrawRighBoundText(const Font& f, const char* txt, Vector2 pos, float fs, float spc = 1.0F, Color tint = WHITE);

} // namespace magique

#endif //MAGIQUE_DRAW_H