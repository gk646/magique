#ifndef MAGIQUE_DRAW_H
#define MAGIQUE_DRAW_H

#include <magique/core/Types.h>
#include <raylib/raylib.h>

namespace magique
{
    // Draws a given texture region
    void DrawRegion(TextureRegion region, float x, float y, Color tint = WHITE, bool flipX = false);

    void DrawRegionEx(TextureRegion region, float x, float y, bool flipX, float rotation);

    // Draws the given frame from the sprite sheet
    void DrawSprite(SpriteSheet sheet, float x, float y, int frame);

} // namespace magique

#endif //MAGIQUE_DRAW_H