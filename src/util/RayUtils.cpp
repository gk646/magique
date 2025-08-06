// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/core/Types.h>
#include <magique/util/RayUtils.h>
#include "external/raylib-compat/rcore_compat.h"

namespace magique
{
    float MeasureTextUpTo(const char* text, const int index, const Font& f, const float fontSize, const float spacing)
    {
        char* nonConstText = const_cast<char*>(text);
        const auto temp = text[index];
        nonConstText[index] = '\0';
        const float ret = MeasureTextEx(f, text, fontSize, spacing).x;
        nonConstText[index] = temp;
        return ret;
    }

    int CountTextUpTo(const char* text, float width, const Font& font, float fontSize, float spacing)
    {
        int size = TextLength(text);                  // Total size in bytes of the text, scanned by codepoints in loop
        float textOffsetY = 0;                        // Offset between lines (on linebreak '\n')
        float textOffsetX = 0.0f;                     // Offset X to next character to draw
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
                textOffsetX += charOff;
            }
            i += codepointByteCount; // Move text bytes counter to next codepoint
        }
        return size;
    }

    float MeasurePixelText(const char* text, const Font& font, int mult)
    {
        return MeasureTextEx(font, text, static_cast<float>(font.baseSize * mult), 1.0F).x;
    }

    float MeasurePixelTextUpTo(const char* text, int index, const Font& font, int mult)
    {
        return MeasureTextUpTo(text, index, font, static_cast<float>(font.baseSize * mult), 1.0F);
    }

    float GetRandomFloat(const float min, const float max)
    {
        constexpr float ACCURACY = 100'000.0F;
        const int minI = static_cast<int>(min * ACCURACY);
        const int maxI = static_cast<int>(max * ACCURACY);
        const auto val = static_cast<float>(GetRandomValue(minI, maxI));
        return val / ACCURACY;
    }

    Vector2 GetCenteredPos(const Rectangle& within, const float width, const float height)
    {
        return Vector2{within.x + ((within.width - width) / 2), within.y + ((within.height - height) / 2)};
    }

    Vector2 GetRectCenter(const Rectangle& rect)
    {
        return Vector2{rect.x + rect.width / 2.0F, rect.y + rect.height / 2.0F};
    }

    Rectangle GetCenteredRect(const Point& center, const float width, const float height)
    {
        return Rectangle{center.x - width / 2.0F, center.y - height / 2.0F, width, height};
    }

    Rectangle GetEnlargedRect(const Rectangle& rect, float width, float height)
    {
        return Rectangle{rect.x - width / 2, rect.y - height / 2, rect.width + width, rect.height + height};
    }

    float GetRoundness(const float radius, const Rectangle& bounds)
    {
        if (bounds.width > bounds.height)
        {
            // radius = (height * roundness) /2
            return (radius * 2.0F) / bounds.height;
        }
        else
        {
            return (radius * 2.0F) / bounds.width;
        }
    }

    static Point prevRes = {};

    void ToggleFullscreenEx()
    {
        if (!IsWindowFullscreen())
        {
            prevRes = {(float)GetScreenWidth(), (float)GetScreenHeight()};
            MaximizeWindow();
            const auto monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        }
        else
        {
            RestoreWindow();
            SetWindowSize((int)prevRes.x, (int)prevRes.y);
        }
        ToggleFullscreen();
    }

    Texture LoadTextureFromMemory(const unsigned char* data, int size, const char* fileType)
    {
        auto img = LoadImageFromMemory(fileType, data, size);
        auto texture = LoadTextureFromImage(img);
        UnloadImage(img);
        return texture;
    }

    void DrawTextureScaled(const Texture& texture, const Rectangle& dest, const Color& tint)
    {
        DrawTexturePro(texture, {0, 0, (float)texture.width, (float)texture.height}, dest, {0, 0}, 0, tint);
    }

    void DrawPixelOutline(const Rectangle& bounds, const Color& outline, const Color& border, const Color& filler,
                          float radius)
    {
        // First lighter outline
        // then dark outline
        // then border

        const auto largeRect = GetEnlargedRect(bounds, 2, 2);
        DrawRectangleRoundedLinesEx(largeRect, GetRoundness(radius + 1, largeRect), 30, 1, filler);
        DrawRectangleRoundedLinesEx(largeRect, GetRoundness(radius, largeRect), 30, 1, outline);
        const auto boundsRound = GetRoundness(radius, bounds);
        DrawRectangleRoundedLinesEx(bounds, boundsRound, 30, 1, border);
    }


} // namespace magique