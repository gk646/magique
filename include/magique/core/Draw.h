#ifndef MAGIQUE_DRAW_H
#define MAGIQUE_DRAW_H

#include <magique/core/Types.h>
#include <raylib/raylib.h>

//-----------------------------------------------
// Draw Module
//-----------------------------------------------
// .....................................................................
// Here are all the magique drawing function that take its custom types
// Generally you cant really use raylibs anymore as texture are stored in atlases
// .....................................................................

namespace magique
{
    // Draws a given texture region
    void DrawRegion(TextureRegion region, float x, float y, Color tint = WHITE, bool flipX = false);

    // Draws the given texture region and scales it according to the destination rect
    // Pass -width to flip the texture horizontally
    void DrawRegionEx(TextureRegion region, Rectangle dest, float rotation, Color tint = WHITE);

    // Draws the given frame from the sprite sheet
    void DrawSprite(SpriteSheet sheet, float x, float y, int frame, Color tint = WHITE);

    // Draws and scales the given frame of the sprite sheet into the destination rect
    void DrawSpriteEx(SpriteSheet sheet, Rectangle dest, int frame, float rotation, Color tint = WHITE);

} // namespace magique

#endif //MAGIQUE_DRAW_H