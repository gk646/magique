#include <cmath>

#include <magique/core/Draw.h>
#include <magique/util/Defines.h>
#include <magique/util/Macros.h>

#include "external/raylib/src/rlgl.h"

#include <magique/assets/types/TileMap.h>


namespace magique
{
    // Passed as value because of low size
    void DrawRegion(TextureRegion region, float x, float y, const bool flipX, const Color tint)
    {
        // Check if the region is valid
        M_ASSERT(region.id > 0, "The texture for this region is invalid");

        x = std::floor(x); // Flooring to avoid texture glitches
        y = std::floor(y);

        const auto texWidth = static_cast<float>(region.width);
        const auto texHeight = static_cast<float>(region.height);

        const auto offsetX = static_cast<float>(region.offX);
        const auto offsetY = static_cast<float>(region.offY);

        if (flipX) [[unlikely]]
        {
            region.width *= -1;
        }

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
        rlVertex2f(x, y);

        // Bottom-left corner for region and quad
        rlTexCoord2f(texCoordLeft, texCoordBottom);
        rlVertex2f(x, y + texHeight);

        // Bottom-right corner for region and quad
        rlTexCoord2f(texCoordRight, texCoordBottom);
        rlVertex2f(x + texWidth, y + texHeight);

        // Top-right corner for region and quad
        rlTexCoord2f(texCoordRight, texCoordTop);
        rlVertex2f(x + texWidth, y);

        rlEnd();
        rlSetTexture(0);
    }

    void DrawSprite(SpriteSheet sheet, float x, float y, int frame, bool flipX, Color tint)
    {
        // Check if the region is valid
        M_ASSERT(sheet.id > 0, "The texture for this region is invalid");
        M_ASSERT(frame >= 0 && frame <= sheet.frames, "Out of bounds frame");

        x = std::floor(x); // Flooring to avoid texture glitches
        y = std::floor(y);

        const auto texWidth = static_cast<float>(sheet.width);
        const auto texHeight = static_cast<float>(sheet.height);

        const auto offsetX = static_cast<float>(sheet.offX + frame * sheet.width);
        const auto offsetY = static_cast<float>(sheet.offY);

        if (flipX) [[unlikely]]
        {
            sheet.width *= -1;
        }

        constexpr auto atlasWidth = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_WIDTH);
        constexpr auto atlasHeight = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_HEIGHT);

        const float texCoordLeft = offsetX / atlasWidth;
        const float texCoordRight = (offsetX + texWidth) / atlasWidth;
        const float texCoordTop = offsetY / atlasHeight;
        const float texCoordBottom = (offsetY + texHeight) / atlasHeight;

        rlSetTexture(sheet.id);
        rlBegin(RL_QUADS);

        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f); // Normal vector pointing towards the viewer

        // Top-left corner for region and quad
        rlTexCoord2f(texCoordLeft, texCoordTop);
        rlVertex2f(x, y);

        // Bottom-left corner for region and quad
        rlTexCoord2f(texCoordLeft, texCoordBottom);
        rlVertex2f(x, y + texHeight);

        // Bottom-right corner for region and quad
        rlTexCoord2f(texCoordRight, texCoordBottom);
        rlVertex2f(x + texWidth, y + texHeight);

        // Top-right corner for region and quad
        rlTexCoord2f(texCoordRight, texCoordTop);
        rlVertex2f(x + texWidth, y);

        rlEnd();
        rlSetTexture(0);
    }



    void DrawTileMap(const TileMap& tileMap, const TileSheet& tileSheet, int layer)
    {
        M_ASSERT(tileMap.layerCount >= layer,"Out of bounds layer!");
        int startIdx = tileMap
    }


} // namespace magique