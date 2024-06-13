#include <cmath>

#include <magique/core/Core.h>
#include <magique/core/Draw.h>
#include <magique/util/Defines.h>
#include <magique/util/Macros.h>
#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSheet.h>

#include "external/raylib/src/rlgl.h"

#include <cxconfig.h>
#include <cxutil/cxtime.h>

namespace magique
{
    void DrawRegion(TextureRegion region, float x, float y, const bool flipX, const Color tint)
    {
        // Check if the region is valid
        M_ASSERT(region.id > 0, "The texture for this region is invalid");

        x = std::floor(x); // Flooring to avoid texture glitches
        y = std::floor(y);

        const auto texWidth = static_cast<float>(region.width);
        const auto texHeight = static_cast<float>(region.height);

        if (flipX) [[unlikely]]
        {
            region.width *= -1;
        }

        constexpr auto atlasWidth = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_WIDTH);
        constexpr auto atlasHeight = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_HEIGHT);

        const float texCoordLeft = static_cast<float>(region.offX) / atlasWidth;
        const float texCoordRight = (static_cast<float>(region.offX) + texWidth) / atlasWidth;
        const float texCoordTop = static_cast<float>(region.offY) / atlasHeight;
        const float texCoordBottom = (static_cast<float>(region.offY) + texHeight) / atlasHeight;

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
        M_ASSERT(frame >= 0 && frame < sheet.frames, "Out of bounds frame");

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
        {
            rlBegin(RL_QUADS);
            {
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
            }
            rlEnd();
        }
        rlSetTexture(0);
    }

    void DrawTileMap(const TileMap& tileMap, const TileSheet& tileSheet, int layer)
    {
        M_ASSERT(tileMap.getLayerCount() >= layer, "Out of bounds layer!");

        const auto cameraBounds = GetCameraNativeBounds();

        const float tileSize = tileSheet.texSize;
        const int mapWidth = tileMap.getWidth() * static_cast<int>(tileSize);
        const int mapHeight = tileMap.getHeight() * static_cast<int>(tileSize);
        const int mapWidthTiles = tileMap.getWidth();

        const int startTileX = std::max(0, static_cast<int>(std::floor(cameraBounds.x / tileSize)));
        const int endTileX =
            std::min(mapWidth, static_cast<int>(std::ceil((cameraBounds.x + cameraBounds.width) / tileSize) + 1));
        const int startTileY = std::max(0, static_cast<int>(std::floor(cameraBounds.y / tileSize)));
        const int endTileY =
            std::min(mapHeight, static_cast<int>(std::ceil((cameraBounds.y + cameraBounds.height) / tileSize) + 1));

        auto* start = tileMap.getLayerStart(layer) + startTileX + startTileY * mapWidthTiles;
        const int diffX = endTileX - startTileX;
        const int diffY = endTileY - startTileY;
        const float startX = static_cast<float>(startTileX) * tileSize;

        float screenY = static_cast<float>(startTileY) * tileSize;

        rlSetTexture(tileSheet.textureID);
        rlBegin(RL_QUADS);
        rlColor4ub(255, 255, 255, 255);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        constexpr auto atlasWidth = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_WIDTH);
        constexpr auto atlasHeight = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_HEIGHT);
        for (int i = 0; i < diffY; ++i)
        {
            float screenX = startX;
            for (int j = 0; j < diffX; ++j)
            {
                const auto [x, y] = tileSheet.getOffset(start[j]);
                const float texCoordLeft = x / atlasWidth;
                const float texCoordRight = (x + tileSize) / atlasWidth;
                const float texCoordTop = y / atlasHeight;
                const float texCoordBottom = (y + tileSize) / atlasHeight;

                rlTexCoord2f(texCoordLeft, texCoordTop);
                rlVertex2f(screenX, screenY);

                rlTexCoord2f(texCoordLeft, texCoordBottom);
                rlVertex2f(screenX, screenY + tileSize);

                rlTexCoord2f(texCoordRight, texCoordBottom);
                rlVertex2f(screenX + tileSize, screenY + tileSize);

                rlTexCoord2f(texCoordRight, texCoordTop);
                rlVertex2f(screenX + tileSize, screenY);

                screenX += tileSize;
            }
            start += mapWidthTiles;
            screenY += tileSize;
        }
        rlEnd();
        rlSetTexture(0);
    }


} // namespace magique