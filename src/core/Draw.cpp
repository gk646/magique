// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <cstdlib>
#include <cctype>

#include <magique/core/Camera.h>
#include <magique/core/Draw.h>
#include <magique/internal/Macros.h>
#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSheet.h>

#include <raylib/rlgl.h> // Has to be here

#include "internal/utils/CollisionPrimitives.h"
#include "external/raylib-compat/rcore_compat.h"

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
        MAGIQUE_ASSERT(tileMap.getTileLayerCount() >= layer, "Out of bounds layer!");

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
        DrawTextEx(f, txt, {std::round(pos.x - width / 2.0F), std::round(pos.y)}, fs, spc, c);
    }

    void DrawRightBoundText(const Font& f, const char* txt, const Vector2 pos, const float fs, const float spc,
                            const Color c)
    {
        const auto width = MeasureTextEx(f, txt, fs, spc).x;
        DrawTextEx(f, txt, {std::round(pos.x - width), std::round(pos.y)}, fs, spc, c);
    }

    void DrawPixelText(const Font& f, const char* txt, Vector2 pos, const int fsm, const Color tint)
    {
        pos.x = std::round(pos.x);
        pos.y = std::round(pos.y);
        DrawTextEx(f, txt, pos, static_cast<float>(f.baseSize * fsm), 1.0F *(float)fsm, tint);
    }

    void DrawCenteredPixelText(const Font& f, const char* txt, const Vector2 pos, const int fsm, const Color tint)
    {
        const auto fs = (float)f.baseSize * fsm;
        const auto width = MeasureTextEx(f, txt, fs, 1.0F).x;
        DrawPixelText(f, txt, {std::round(pos.x - width / 2.0F), std::round(pos.y)}, fsm, tint);
    }

    void DrawRightBoundPixelText(const Font& f, const char* txt, Vector2 pos, int fsm, Color tint)
    {
        const auto fs = (float)f.baseSize * fsm;
        const auto width = MeasureTextEx(f, txt, fs, (float)fsm).x;
        DrawPixelText(f, txt, {pos.x - width, pos.y}, fsm, tint);
    }

    int DrawTextUpTo(const Font& font, const char* text, Vector2 position, float fontSize, float width, Color tint)
    {
        int size = TextLength(text); // Total size in bytes of the text, scanned by codepoints in loop

        float spacing = 1.0F;
        float textOffsetY = 0;    // Offset between lines (on linebreak '\n')
        float textOffsetX = 0.0f; // Offset X to next character to draw

        float scaleFactor = fontSize / font.baseSize; // Character quad scaling factor

        for (int i = 0; i < size;)
        {
            // Get next codepoint from byte string and glyph index in font
            int codepointByteCount = 0;
            int codepoint = GetCodepointNext(&text[i], &codepointByteCount);
            int index = GetGlyphIndex(font, codepoint);

            if (codepoint == '\n')
            {
                // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
                textOffsetY += (fontSize + GetTextLineSpacing());
                textOffsetX = 0.0f;
            }
            else
            {
                float charOff = 0.0F;
                if (font.glyphs[index].advanceX == 0)
                    charOff = ((float)font.recs[index].width * scaleFactor + spacing);
                else
                    charOff = ((float)font.glyphs[index].advanceX * scaleFactor + spacing);

                if (charOff + textOffsetX >= width)
                    return i;

                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint, Vector2{position.x + textOffsetX, position.y + textOffsetY},
                                      fontSize, tint);
                }

                textOffsetX += charOff;
            }

            i += codepointByteCount; // Move text bytes counter to next codepoint
        }
        return size;
    }

    void DrawPixelTextNumbers(const Font& font, const char* text, Vector2 position, int fsm, Color textColor,
                              Color numberColor)
    {
        int size = TextLength(text); // Total size in bytes of the text, scanned by codepoints in loop
        float spacing = (float)fsm;
        float fontSize = (float)font.baseSize * (float)fsm;
        float textOffsetY = 0;    // Offset between lines (on linebreak '\n')
        float textOffsetX = 0.0f; // Offset X to next character to draw

        float scaleFactor = fontSize / font.baseSize; // Character quad scaling factor

        for (int i = 0; i < size;)
        {
            // Get next codepoint from byte string and glyph index in font
            int codepointByteCount = 0;
            int codepoint = GetCodepointNext(&text[i], &codepointByteCount);
            int index = GetGlyphIndex(font, codepoint);

            if (codepoint == '\n')
            {
                // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
                textOffsetY += (fontSize + GetTextLineSpacing());
                textOffsetX = 0.0f;
            }
            else
            {
                const Color& txtColor = (isdigit(codepoint) != 0) ? numberColor : textColor;
                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint, Vector2{position.x + textOffsetX, position.y + textOffsetY},
                                      fontSize, txtColor);
                }

                if (font.glyphs[index].advanceX == 0)
                    textOffsetX += ((float)font.recs[index].width * scaleFactor + spacing);
                else
                    textOffsetX += ((float)font.glyphs[index].advanceX * scaleFactor + spacing);
            }

            i += codepointByteCount; // Move text bytes counter to next codepoint
        }
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