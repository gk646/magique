#include <cmath>

#include <magique/core/Core.h>
#include <magique/core/Draw.h>
#include <magique/internal/Macros.h>
#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSheet.h>

#include <raylib/rlgl.h> // Has to be here

#include "internal/headers/CollisionPrimitives.h"

constexpr auto ATLAS_WIDTH = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_WIDTH);
constexpr auto ATLAS_HEIGHT = static_cast<float>(MAGIQUE_TEXTURE_ATLAS_HEIGHT);

namespace magique
{
    void DrawRegion(TextureRegion region, const float x, const float y, const bool flipX, const Color tint)
    {
        // Check if the region is valid
        M_ASSERT(region.id > 0, "The texture for this region is invalid");

        const auto texWidth = static_cast<float>(region.width);
        const auto texHeight = static_cast<float>(region.height);

        if (flipX) [[unlikely]]
        {
            region.width *= -1;
        }

        const float texCoordLeft = static_cast<float>(region.offX) / ATLAS_WIDTH;
        const float texCoordRight = (static_cast<float>(region.offX) + texWidth) / ATLAS_WIDTH;
        const float texCoordTop = static_cast<float>(region.offY) / ATLAS_HEIGHT;
        const float texCoordBottom = (static_cast<float>(region.offY) + texHeight) / ATLAS_HEIGHT;

        rlSetTexture(region.id);
        rlBegin(RL_QUADS);

        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f);

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

    void DrawRegionEx(TextureRegion region, const float x, const float y, const float rotation, const bool flipX,
                      const Color tint)
    {
        // Check if the region is valid
        M_ASSERT(region.id > 0, "The texture for this region is invalid");

        const auto texWidth = static_cast<float>(region.width);
        const auto texHeight = static_cast<float>(region.height);

        if (flipX) [[unlikely]]
        {
            region.width *= -1;
        }

        const float texCoordLeft = static_cast<float>(region.offX) / ATLAS_WIDTH;
        const float texCoordRight = (static_cast<float>(region.offX) + texWidth) / ATLAS_WIDTH;
        const float texCoordTop = static_cast<float>(region.offY) / ATLAS_HEIGHT;
        const float texCoordBottom = (static_cast<float>(region.offY) + texHeight) / ATLAS_HEIGHT;

        rlSetTexture(region.id);
        rlBegin(RL_QUADS);

        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        if (rotation != 0)
        {
            // Rotation in radians
            const float cosTheta = std::cos(rotation * DEG2RAD);
            const float sinTheta = std::sin(rotation * DEG2RAD);

            // Center of rotation
            const float cx = x + texWidth / 2;
            const float cy = y + texHeight / 2;

            auto rotate = [&](const float px, const float py)
            {
                const float dx = px - cx;
                const float dy = py - cy;
                return std::make_pair(cx + dx * cosTheta - dy * sinTheta, cy + dx * sinTheta + dy * cosTheta);
            };

            // Top-left corner for region and quad
            auto [tx1, ty1] = rotate(x, y);
            rlTexCoord2f(texCoordLeft, texCoordTop);
            rlVertex2f(tx1, ty1);

            // Bottom-left corner for region and quad
            auto [tx2, ty2] = rotate(x, y + texHeight);
            rlTexCoord2f(texCoordLeft, texCoordBottom);
            rlVertex2f(tx2, ty2);

            // Bottom-right corner for region and quad
            auto [tx3, ty3] = rotate(x + texWidth, y + texHeight);
            rlTexCoord2f(texCoordRight, texCoordBottom);
            rlVertex2f(tx3, ty3);

            // Top-right corner for region and quad
            auto [tx4, ty4] = rotate(x + texWidth, y);
            rlTexCoord2f(texCoordRight, texCoordTop);
            rlVertex2f(tx4, ty4);
        }
        else
        {
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
        rlSetTexture(0);
    }

    void DrawRegionPro(const TextureRegion region, Rectangle dest,const float rotation,const float rotX,const float rotY, Color tint)
    {
        float sinRotation = sinf(rotation * DEG2RAD);
        float cosRotation = cosf(rotation * DEG2RAD);

        float pivotX = dest.x + rotX;
        float pivotY = dest.y + rotY;

        float offsetX[4] = {-rotX, -rotX, dest.width - rotX, dest.width - rotX};
        float offsetY[4] = {-rotY, dest.height - rotY, dest.height - rotY, -rotY};

        rlSetTexture(region.id);
        rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        float texCoords[4][2] = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}};

        for (int i = 0; i < 4; ++i)
        {
            float rotatedX = cosRotation * offsetX[i] - sinRotation * offsetY[i] + pivotX;
            float rotatedY = sinRotation * offsetX[i] + cosRotation * offsetY[i] + pivotY;

            rlTexCoord2f(texCoords[i][0], texCoords[i][1]);
            rlVertex2f(rotatedX, rotatedY);
        }

        rlEnd();
        rlSetTexture(0);
    }

    void DrawSprite(SpriteSheet sheet, float x, float y, const int frame, const bool flipX, const Color tint)
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

    void DrawTileMap(const TileMap& tileMap, const TileSheet& tileSheet, const int layer)
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

    void DrawCenteredText(const Font& f, const char* txt, const Vector2 pos, const float fs, const float spc,
                          const Color c)
    {
        const auto width = MeasureTextEx(f, txt, fs, spc).x;
        DrawTextEx(f, txt, {pos.x - width / 2.0F, pos.y}, fs, spc, c);
    }

    void DrawRighBoundText(const Font& f, const char* txt, const Vector2 pos, const float fs, const float spc,
                           const Color c)
    {
        const auto width = MeasureTextEx(f, txt, fs, spc).x;
        DrawTextEx(f, txt, {pos.x - width, pos.y}, fs, spc, c);
    }

    void DrawCapsule2D(const float x, const float y, float radius,const float height, const Color tint)
    {
        if (radius <= 0.0f)
            radius = 0.1f;

        const Vector2 topCenter = {x + radius, y + radius};
        const Vector2 bottomCenter = {x + radius, y + height - radius};
        DrawCircleSector(topCenter, radius, 180.0f, 360.0f, 32, tint);
        DrawCircleSector(bottomCenter, radius, 0.0f, 180.0f, 32, tint);
        DrawRectangleRec({x, y + radius, 2 * radius, height - 2 * radius}, tint);
    }

    void DrawCapsule2DLines(const float x, const float y, float radius,const float height, const Color tint)
    {
        if (radius <= 0.0f)
            radius = 0.1f;

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
        float txs[4] = {0, p2.x-p1.x, p3.x-p1.x,0};
        float tys[4] = {0, p2.y-p1.y, p3.y-p1.y,0};
        RotatePoints4(p1.x, p1.y, txs, tys, rot, pivotX, pivotY);


        DrawTriangle({txs[0], tys[0]}, {txs[1], tys[1]}, {txs[2], tys[2]}, color);
    }

    void DrawTriangleLinesRot(const Vector2 p1, const Vector2 p2, const Vector2 p3, const float rot, const float pivotX,
                              const float pivotY, const Color color)
    {
        float txs[4] = {0, p2.x-p1.x, p3.x-p1.x,0};
        float tys[4] = {0, p2.y-p1.y, p3.y-p1.y,0};
        RotatePoints4(p1.x, p1.y, txs, tys, rot, pivotX, pivotY);
        DrawTriangleLines({txs[0], tys[0]}, {txs[1], tys[1]}, {txs[2], tys[2]}, color);
    }

} // namespace magique