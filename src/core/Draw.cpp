#include <cassert>

#include <magique/core/Draw.h>
#include <magique/core/Defines.h>

#include <raylib/raylib.h>

#include "external/raylib/src/rlgl.h"

namespace magique
{

    void DrawRegion(TextureRegion region, const float x, float y, const Color tint, const bool flipX)
    {
        // Check if the region is valid
        assert(region.id > 0);

        const auto texWidth = static_cast<float>(region.width);
        const auto texHeight = static_cast<float>(region.height);

        const auto offsetX = static_cast<float>(region.offX);
        const auto offsetY = static_cast<float>(region.offY);

        if (flipX)
        {
            std::swap(region.offX, region.width);
        }

        const Vector2 topLeft = {x, y};
        const Vector2 bottomLeft = {x, y + texHeight};
        const Vector2 bottomRight = {x + texWidth, y + texHeight};
        const Vector2 topRight = {x + texWidth, y};

        constexpr auto atlasWidth = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_WIDTH);
        constexpr auto atlasHeight = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_HEIGHT);

        const float texCoordLeft = offsetX / atlasWidth;
        const float texCoordRight = (offsetX + texWidth) / atlasWidth;
        const float texCoordTop = offsetY / atlasHeight;
        const float texCoordBottom = (offsetY + texHeight) / atlasHeight;

        rlSetTexture(region.id);
        rlBegin(RL_QUADS);

        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f); // Normal vector pointing towards the viewer

        // Top-left corner for region and quad
        rlTexCoord2f(texCoordLeft, texCoordTop);
        rlVertex2f(topLeft.x, topLeft.y);

        // Bottom-left corner for region and quad
        rlTexCoord2f(texCoordLeft, texCoordBottom);
        rlVertex2f(bottomLeft.x, bottomLeft.y);

        // Bottom-right corner for region and quad
        rlTexCoord2f(texCoordRight, texCoordBottom);
        rlVertex2f(bottomRight.x, bottomRight.y);

        // Top-right corner for region and quad
        rlTexCoord2f(texCoordRight, texCoordTop);
        rlVertex2f(topRight.x, topRight.y);

        rlEnd();
        rlSetTexture(0);
    }


    void DrawSprite(SpriteSheet sheet, float x, float y, int frame) {}


} // namespace magique