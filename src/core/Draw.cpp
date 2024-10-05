#include <cmath>

#include <magique/core/Core.h>
#include <magique/core/Draw.h>
#include <magique/internal/Macros.h>
#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSheet.h>

#include <raylib/rlgl.h> // Has to be here

#include "internal/headers/CollisionPrimitives.h"

constexpr auto ATLAS_WIDTH = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_SIZE);
constexpr auto ATLAS_HEIGHT = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_SIZE);

namespace magique
{
    void DrawRegion(const TextureRegion region, const float x, const float y, const bool flipX, const Color tint)
    {
        // Check if the region is valid
        MAGIQUE_ASSERT(region.id > 0, "The texture for this region is invalid");

        const auto texWidth = static_cast<float>(region.width);
        const auto texHeight = static_cast<float>(region.height);

        const float texLeft = (static_cast<float>(region.offX) + (flipX ? texWidth : 0)) / ATLAS_WIDTH;
        const float texRight = (static_cast<float>(region.offX) + (flipX ? 0 : texWidth)) / ATLAS_WIDTH;
        const float texTop = static_cast<float>(region.offY) / ATLAS_HEIGHT;
        const float texBottom = (static_cast<float>(region.offY) + texHeight) / ATLAS_HEIGHT;

        rlSetTexture(region.id);
        rlBegin(RL_QUADS);

        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        // Top-left corner for region and quad
        rlTexCoord2f(texLeft, texTop);
        rlVertex2f(x, y);

        // Bottom-left corner for region and quad
        rlTexCoord2f(texLeft, texBottom);
        rlVertex2f(x, y + texHeight);

        // Bottom-right corner for region and quad
        rlTexCoord2f(texRight, texBottom);
        rlVertex2f(x + texWidth, y + texHeight);

        // Top-right corner for region and quad
        rlTexCoord2f(texRight, texTop);
        rlVertex2f(x + texWidth, y);

        rlEnd();
        rlSetTexture(0);
    }

    void DrawRegionPro(TextureRegion region, Rectangle dest, const float rotation, const Point anchor, const Color tint)
    {
        // Check if the region is valid
        MAGIQUE_ASSERT(region.id > 0, "The texture for this region is invalid");

        const auto texWidth = static_cast<float>(region.width);
        const auto texHeight = static_cast<float>(region.height);

        const float texLeft = (static_cast<float>(region.offX) + (dest.width < 0 ? texWidth : 0)) / ATLAS_WIDTH;
        const float texRight = (static_cast<float>(region.offX) + (dest.width < 0 ? 0 : texWidth)) / ATLAS_WIDTH;
        const float texTop = (static_cast<float>(region.offY) + (dest.height < 0 ? texHeight : 0)) / ATLAS_HEIGHT;
        const float texBottom = (static_cast<float>(region.offY) + (dest.height < 0 ? 0 : texHeight)) / ATLAS_HEIGHT;

        dest.width = std::abs(dest.width);
        dest.height = std::abs(dest.height);

        rlSetTexture(region.id);
        rlBegin(RL_QUADS);

        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        if (rotation != 0) [[unlikely]]
        {
            float pxs[4] = {0, 0, dest.width, dest.width};
            float pys[4] = {0, dest.height, dest.height, 0};

            RotatePoints4(dest.x, dest.y, pxs, pys, rotation, anchor.x, anchor.y);

            rlTexCoord2f(texLeft, texTop);
            rlVertex2f(pxs[0], pys[0]);

            rlTexCoord2f(texLeft, texBottom);
            rlVertex2f(pxs[1], pys[1]);

            rlTexCoord2f(texRight, texBottom);
            rlVertex2f(pxs[2], pys[2]);

            rlTexCoord2f(texRight, texTop);
            rlVertex2f(pxs[3], pys[3]);
        }
        else
        {
            // Top-left corner for region and quad
            rlTexCoord2f(texLeft, texTop);
            rlVertex2f(dest.x, dest.y);

            // Bottom-left corner for region and quad
            rlTexCoord2f(texLeft, texBottom);
            rlVertex2f(dest.x, dest.y + dest.height);

            // Bottom-right corner for region and quad
            rlTexCoord2f(texRight, texBottom);
            rlVertex2f(dest.x + dest.width, dest.y + dest.height);

            // Top-right corner for region and quad
            rlTexCoord2f(texRight, texTop);
            rlVertex2f(dest.x + dest.width, dest.y);
        }

        rlEnd();
        rlSetTexture(0);
    }

    void DrawSprite(SpriteSheet sheet, float x, float y, const int frame, const bool flipX, const Color tint)
    {
        // Check if the region is valid
        MAGIQUE_ASSERT(sheet.id > 0, "The texture for this region is invalid");
        MAGIQUE_ASSERT(frame >= 0 && frame < sheet.frames, "Out of bounds frame");

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

        constexpr auto atlasWidth = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_SIZE);
        constexpr auto atlasHeight = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_SIZE);

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

    void DrawTileMap(const TileMap& tileMap, const TileSheet& tileSheet, const int layer)
    {
        MAGIQUE_ASSERT(tileMap.getLayerCount() >= layer, "Out of bounds layer!");

        const auto cBounds = GetCameraNativeBounds(); // Camera bounds
        const float tileSize = tileSheet.getTextureSize();
        const int mWidth = tileMap.getWidth();
        const int mHeight = tileMap.getHeight();

        const int startTileX = std::max(0, (int)std::floor(cBounds.x / tileSize));
        const int endTileX = std::min(mWidth, (int)std::ceil((cBounds.x + cBounds.width) / tileSize));
        const int startTileY = std::max(0, (int)std::floor(cBounds.y / tileSize));
        const int endTileY = std::min(mHeight, (int)std::ceil((cBounds.y + cBounds.height) / tileSize));

        auto* start = tileMap.getLayerData(layer) + startTileX + startTileY * mWidth;
        const int diffX = endTileX - startTileX;
        const int diffY = endTileY - startTileY;
        const float startX = static_cast<float>(startTileX) * tileSize;

        float screenY = static_cast<float>(startTileY) * tileSize;

        rlSetTexture(tileSheet.getTextureID());
        rlBegin(RL_QUADS);
        rlColor4ub(255, 255, 255, 255);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        constexpr auto atlasWidth = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_SIZE);
        constexpr auto atlasHeight = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_SIZE);
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
            start += mWidth;
            screenY += tileSize;
        }
        rlEnd();
        rlSetTexture(0);
    }

    void DrawCenteredText(const Font& f, const char* txt, const Vector2 pos, const float fs, const float spc,
                          const Color c)
    {
        const auto width = MeasureTextEx(f, txt, fs, spc).x;
        DrawTextEx(f, txt, {pos.x - width / 2.0F, pos.y}, fs, spc, c);
    }

    void DrawRightBoundText(const Font& f, const char* txt, const Vector2 pos, const float fs, const float spc,
                            const Color c)
    {
        const auto width = MeasureTextEx(f, txt, fs, spc).x;
        DrawTextEx(f, txt, {pos.x - width, pos.y}, fs, spc, c);
    }

    void DrawCapsule2D(const float x, const float y, const float radius, const float height, const Color tint)
    {
        MAGIQUE_ASSERT(radius > 0, "Math error. Radius is 0");
        const Vector2 topCenter = {x + radius, y + radius};
        const Vector2 bottomCenter = {x + radius, y + height - radius};
        DrawCircleSector(topCenter, radius, 180.0f, 360.0f, 32, tint);
        DrawCircleSector(bottomCenter, radius, 0.0f, 180.0f, 32, tint);
        DrawRectangleRec({x, y + radius, 2 * radius, height - 2 * radius}, tint);
    }

    void DrawCapsule2DLines(const float x, const float y, const float radius, const float height, const Color tint)
    {
        MAGIQUE_ASSERT(radius > 0, "Math error. Radius is 0");
        const Vector2 topCenter = {x + radius, y + radius};
        const Vector2 bottomCenter = {x + radius, y + height - radius};
        DrawCircleSectorLines(topCenter, radius, 180.0F, 360.0F, 32, tint);
        DrawCircleSectorLines(bottomCenter, radius, 0.0F, 180.0F, 32, tint);

        DrawLineV({x, y + radius}, {x, y + height - radius}, tint);
        DrawLineV({x + radius * 2, y + radius}, {x + radius * 2, y + height - radius}, tint);
    }

    void DrawRectangleLinesRot(const Rectangle& rect, const float rotation, const float pivotX, const float pivotY,
                               const Color color)
    {
        float pxs[4] = {0, rect.width, rect.width, 0}; // top-left // top-right// bottom-right// bottom-left
        float pys[4] = {0, 0, rect.height, rect.height};
        RotatePoints4(rect.x, rect.y, pxs, pys, rotation, pivotX, pivotY);

        for (int i = 0; i < 4; ++i)
        {
            const int nextI = (i + 1) % 4;
            DrawLineV({pxs[i], pys[i]}, {pxs[nextI], pys[nextI]}, color);
        }
    }

    void DrawTriangleRot(const Vector2 p1, const Vector2 p2, const Vector2 p3, const float rot, const float pivotX,
                         const float pivotY, const Color color)
    {
        float txs[4] = {0, p2.x - p1.x, p3.x - p1.x, 0};
        float tys[4] = {0, p2.y - p1.y, p3.y - p1.y, 0};
        RotatePoints4(p1.x, p1.y, txs, tys, rot, pivotX, pivotY);


        DrawTriangle({txs[0], tys[0]}, {txs[1], tys[1]}, {txs[2], tys[2]}, color);
    }

    void DrawTriangleLinesRot(const Vector2 p1, const Vector2 p2, const Vector2 p3, const float rot, const float pivotX,
                              const float pivotY, const Color color)
    {
        float txs[4] = {0, p2.x - p1.x, p3.x - p1.x, 0};
        float tys[4] = {0, p2.y - p1.y, p3.y - p1.y, 0};
        RotatePoints4(p1.x, p1.y, txs, tys, rot, pivotX, pivotY);
        DrawTriangleLines({txs[0], tys[0]}, {txs[1], tys[1]}, {txs[2], tys[2]}, color);
    }

} // namespace magique