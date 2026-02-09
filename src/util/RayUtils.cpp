// SPDX-License-Identifier: zlib-acknowledgement
#include <algorithm>
#include <cmath>

#include <raylib/raylib.h>
#include <magique/core/Types.h>
#include <magique/util/RayUtils.h>
#include "external/raylib-compat/rcore_compat.h"
#include "magique/ui/UI.h"

namespace magique
{
    float MeasureTextUpTo(const char* text, const int index, const Font& f, const float fontSize, const float spacing)
    {
        char* nonConstText = const_cast<char*>(text);
        const auto temp = text[index];
        nonConstText[index] = '\0';
        const float ret = MeasureTextEx(f, text, fontSize, spacing).x;
        nonConstText[index] = temp;
        return std::floor(ret);
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
        return Vector2{std::floor(within.x + ((within.width - width) / 2.0F)),
                       std::floor(within.y + ((within.height - height) / 2.0F))};
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
        return Rectangle{rect.x - (width / 2), rect.y - (height / 2), rect.width + width, rect.height + height};
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

    void DrawRenderTexture(const RenderTexture& texture, const Vector2& pos, float scale, const Color& tint)
    {
        DrawTexturePro(texture.texture, {0, 0, (float)texture.texture.width, -(float)texture.texture.height},
                       {pos.x, pos.y, (float)texture.texture.width * scale, (float)texture.texture.height * scale}, {},
                       0, tint);
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

    void DrawPixelBorder(const Rectangle& bounds, const Color& outline, const Color& border)
    {
        auto enlargedRect = GetEnlargedRect(bounds, 2, 2);
        DrawRectangleLinesEx(enlargedRect, 1, outline);
        DrawRectangleLinesEx(bounds, 1, border);
    }

    bool CheckCollisionMouseRect(const Rectangle& bounds) { return CheckCollisionPointRec(GetMousePosition(), bounds); }

    void DrawCenteredTextureV(const Texture& texture, const Vector2& pos, const Color& tint)
    {
        auto center = pos;
        center.x -= (float)texture.width / 2.0F;
        center.x = std::floor(center.x);
        DrawTextureV(texture, center, tint);
    }

    void DrawFilledRect(const Rectangle& bounds, const float fillPercent, const Direction dir, const Color& tint)
    {
        Rectangle filledBounds = {bounds.x, bounds.y, bounds.width, bounds.height};
        if (dir == Direction::UP)
        {
            filledBounds.height *= fillPercent;
            filledBounds.y = bounds.y - filledBounds.height + bounds.height;
        }
        else if (dir == Direction::DOWN)
        {
            filledBounds.height *= fillPercent;
        }
        else if (dir == Direction::LEFT)
        {
            filledBounds.width *= fillPercent;
            filledBounds.x = bounds.x - filledBounds.width + bounds.width;
        }
        else if (dir == Direction::RIGHT)
        {
            filledBounds.width *= fillPercent;
        }
        DrawRectangleRec(filledBounds, tint);
    }

    void DrawTextCenteredRect(const Font& fnt, const char* txt, float fs, const Rectangle& bounds, float spacing,
                              const Color& tint)
    {
        const auto dims = MeasureTextEx(fnt, txt, fs, spacing);
        const auto center = GetCenteredPos(bounds, dims.x, dims.y);
        DrawTextEx(fnt, txt, center, fs, spacing, tint);
    }

    void DrawRectangleShaded(const Rectangle& bounds, const Color& tint, const Color& shade, float shadeMult)
    {
        DrawRectangleRec(bounds, tint);
        const auto shadeRect = Rectangle{bounds.x, bounds.y + bounds.height, bounds.width, bounds.height * shadeMult};
        DrawRectangleRec(shadeRect, shade);
    }

    void DrawRectFrame(const Rectangle& bounds, const Color& tint)
    {
        const float x = bounds.x;
        const float y = bounds.y;
        const float width = bounds.width;
        const float height = bounds.height;

#ifdef _MSC_VER
        const Vector2 topStart = {x, y};
        const Vector2 topEnd = {x + width - 2, y};

        const Vector2 bottomStart = {x, y + height - 1};
        const Vector2 bottomEnd = {x + width - 2, y + height - 1};

        const Vector2 leftStart = {x, y};
        const Vector2 leftEnd = {x, y + height - 2};

        const Vector2 rightStart = {x + width - 1, y};
        const Vector2 rightEnd = {x + width - 1, y + height - 2};
#else
        const Vector2 topStart = {x + 1, y};
        const Vector2 topEnd = {x + width - 1, y};

        const Vector2 bottomStart = {x + 1, y + height - 1};
        const Vector2 bottomEnd = {x + width - 1, y + height - 1};

        const Vector2 leftStart = {x + 0.01F, y + 1};
        const Vector2 leftEnd = {x + 0.01F, y + height - 1};

        const Vector2 rightStart = {x + width, y + 1};
        const Vector2 rightEnd = {x + width, y + height - 1};
#endif

        DrawLineV(topStart, topEnd, tint);
        DrawLineV(bottomStart, bottomEnd, tint);
        DrawLineV(leftStart, leftEnd, tint);
        DrawLineV(rightStart, rightEnd, tint);
    }

    void DrawRectFrameFilled(const Rectangle& bounds, const Color& fill, const Color& outline)
    {
        const auto final = RectFloor(bounds);
        DrawRectangleRec(GetEnlargedRect(final, -2, -2), fill);
        DrawRectFrame(final, outline);
    }

    void DrawTruePixelartScale(RenderTexture texture)
    {
        auto canvas = Point{(float)texture.texture.width, (float)texture.texture.height};
        const auto display = Point{(float)GetScreenWidth(), (float)GetScreenHeight()};
        auto scale = display / canvas;
        if (scale <= 0)
        {
            scale = {1, 1};
        }
        // Use for both axis the smallest common scalar
        scale.y = std::min(scale.x, scale.y);
        scale.x = std::min(scale.y, scale.x);
        canvas *= scale;
        canvas.floor();
        SetMouseScale(1 / scale.x, 1 / scale.y);
        const auto drawPos = GetCenteredPos({0, 0, display.x, display.y}, canvas.x, canvas.y);
        SetMouseOffset((int)-drawPos.x, (int)-drawPos.y);
        DrawRenderTexture(texture, drawPos, scale.x, WHITE);
    }

    Rectangle RectMove(const Rectangle& original, float x, float y)
    {
        return {original.x + x, original.y + y, original.width, original.height};
    }

    Rectangle RectFloor(const Rectangle& rect)
    {
        Rectangle floored = rect;
        floored.x = std::floor(floored.x);
        floored.y = std::floor(floored.y);
        floored.width = std::floor(floored.width);
        floored.height = std::floor(floored.height);
        return floored;
    }

    Point MouseDragger::update(Camera2D& camera, float zoomMult, float min, float max)
    {
        Point newTarget = {camera.target.x, camera.target.y};
        const auto move = LayeredInput::GetIsMouseConsumed() ? 0.0F : GetMouseWheelMove();
        if (move > 0)
        {
            resetDragPos(camera);
            camera.zoom = std::min(camera.zoom + move * zoomMult, max);
        }
        else if (move < 0)
        {
            resetDragPos(camera);
            camera.zoom = std::max(camera.zoom + move * zoomMult, min);
        }

        if (GetDragStartPosition() == -1)
        {
            dragStartScreen = GetMousePos();
            dragStartWorld = camera.target;
        }
        else
        {
            auto diff = (dragStartScreen - GetMousePos()) / camera.zoom;
            newTarget = dragStartWorld + diff.floor();
            newTarget.floor();
        }
        return newTarget;
    }

    Point MouseDragger::getDragOffset(Camera2D& camera) const { return (dragStartScreen - GetMousePos()) / camera.zoom; }

    Point MouseDragger::getDragStartWorld() const { return dragStartWorld; }

    void MouseDragger::resetDragPos(Camera2D& camera)
    {
        // When dragging we have to reset drag position when zooming
        // otherwise target jumps as diff is made with different zoom levels
        if (GetDragStartPosition() == -1)
        {
            return;
        }

        const auto diff = dragStartScreen - GetMousePos();
        auto newTarget = dragStartWorld + (diff / camera.zoom);
        camera.target.x = newTarget.x;
        camera.target.y = newTarget.y;

        dragStartWorld = newTarget;
        dragStartScreen = GetMousePos();
    }

    Point GetGamePadLeftStick(int gamepad)
    {
        Point p;
        p.x = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
        p.y = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);
        return p;
    }

    Point GetGamePadRightStick(int gamepad)
    {
        Point p;
        p.x = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X);
        p.y = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y);
        return p;
    }


    bool IsUsingGamepad() { return false; }

} // namespace magique
