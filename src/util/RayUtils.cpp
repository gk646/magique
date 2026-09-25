// SPDX-License-Identifier: zlib-acknowledgement
#include <algorithm>
#include <cmath>

#include <raylib/raylib.h>
#include <magique/core/Types.h>
#include <magique/util/RayUtils.h>
#include <magique/ui/UI.h>
#include <magique/core/Camera.h>

#ifdef USING_SDL3_PROJECT
#include "SDL3/SDL.h"
#else
#include "external/glfw/include/GLFW/glfw3.h"
#endif

namespace magique
{
    Point GetMousePos() { return Point{GetMousePosition()}.floor(); }

    Texture LoadTextureFromMemory(const unsigned char* data, int size, const char* fileType)
    {
        auto img = LoadImageFromMemory(fileType, data, size);
        auto texture = LoadTextureFromImage(img);
        UnloadImage(img);
        return texture;
    }

    void DrawRenderTexture(const RenderTexture& texture, Point pos, Point scale, Color tint)
    {
        DrawTexturePro(texture.texture, {0, 0, (float)texture.texture.width, -(float)texture.texture.height},
                       {pos.x, pos.y, (float)texture.texture.width * scale.x, (float)texture.texture.height * scale.y},
                       {}, 0, tint);
    }

    void DrawRenderTextureEx(const RenderTexture& texture, Rect dest, float rot, Color tint)
    {
        DrawTexturePro(texture.texture, {0, 0, (float)texture.texture.width, -(float)texture.texture.height}, dest, {},
                       rot, tint);
    }

    float MeasureTextUpTo(std::string_view text, const int index, const Font& f, const float fontSize, const float spacing)
    {
        thread_local std::string buffer;
        buffer = text.subview(0, std::max(index, 0));
        const float ret = MeasureTextEx(f, buffer.c_str(), fontSize, spacing).x;
        return std::floor(ret);
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


    void DrawPixelOutline(const Rectangle& bounds, const Color& outline, const Color& border, const Color& filler,
                          float radius)
    {
        // First lighter outline
        // then dark outline
        // then border
        const auto largeRect = Rect{bounds}.enlarge(2);
        DrawRectangleRoundedLinesEx(largeRect, GetRoundness(radius + 1, largeRect), 30, 1, filler);
        DrawRectangleRoundedLinesEx(largeRect, GetRoundness(radius, largeRect), 30, 1, outline);
        const auto boundsRound = GetRoundness(radius, bounds);
        DrawRectangleRoundedLinesEx(bounds, boundsRound, 30, 1, border);
    }

    void DrawPixelBorder(const Rect& bounds, const Color& outline, const Color& border)
    {
        auto enlargedRect = Rect{bounds}.enlarge(2);
        DrawRectangleLinesEx(enlargedRect, 1, outline);
        DrawRectangleLinesEx(bounds, 1, border);
    }

    bool CheckCollisionMouseRect(const Rectangle& bounds) { return CheckCollisionPointRec(GetMousePosition(), bounds); }

    void DrawRectFilled(const Rectangle& bounds, const float fillPercent, const Direction dir, Color tint)
    {
        DrawRectangleRec(Rect::Filled(bounds, fillPercent, dir), tint);
    }

    void DrawRectFrame(const Rect& bounds, const Color& tint)
    {
        const auto [x, y, width, height] = bounds;
        const Point topStart = {x + 1.0F, y};
        const Point topEnd = {x + width - 1.0F, y};

        const Point bottomStart = {x + 1.0F, y + height - 1.0F};
        const Point bottomEnd = {x + width - 1.0F, y + height - 1.0F};

        const Point leftStart = {x + 0.01F, y + 1.0F};
        const Point leftEnd = {x + 0.01F, y + height - 1.0F};

        const Point rightStart = {x + width, y + 1.0F};
        const Point rightEnd = {x + width, y + height - 1.0F};

        DrawLineV(topStart.floored(), topEnd, tint);
        DrawLineV(bottomStart.floored(), bottomEnd, tint);
        DrawLineV(leftStart.floored(), leftEnd, tint);
        DrawLineV(rightStart.floored(), rightEnd, tint);
    }

    void DrawRectFrameFilled(const Rect& bounds, const Color& fill, const Color& outline, float fillRatio, Direction dir)
    {
        auto fillRect = Rect::Filled(bounds.shrink(2.0F), fillRatio, dir).floor();
        const auto outRect = Rect::Filled(bounds, fillRatio, dir).floor();
        fillRect.width = std::min(outRect.width - 1, fillRect.width);

        DrawRectangleRec(fillRect, fill);
        if (outRect.width > 1)
            DrawRectFrame(outRect, outline);
    }

    void SetCursorImage(Image img, Point anchor)
    {
#ifdef USING_SDL3_PROJECT
        SDL_Surface* surface =
            SDL_CreateSurfaceFrom(img.width, img.height, SDL_PIXELFORMAT_RGBA32, img.data, img.width * 4);
        if (surface)
        {
            SDL_Cursor* cursor = SDL_CreateColorCursor(surface, (int)anchor.x, (int)anchor.y);
            if (cursor)
            {
                SDL_SetCursor(cursor);
            }
            SDL_DestroySurface(surface);
        }
#else
        GLFWimage glfwImage;
        glfwImage.width = img.width;
        glfwImage.height = img.height;
        glfwImage.pixels = (unsigned char*)img.data;
        auto* cursor = glfwCreateCursor(&glfwImage, 10, 4);
        if (cursor != nullptr)
        {
            glfwSetCursor(glfwGetCurrentContext(), cursor);
        }
#endif
    }

    const std::vector<int>& GetMonitors()
    {
        thread_local std::vector<int> LIST;
        LIST.clear();
#ifdef USING_SDL3_PROJECT
        int monitorCount = 0;
        SDL_DisplayID* displays = SDL_GetDisplays(&monitorCount);
        for (int i = 0; i < monitorCount; i++)
        {
            LIST.push_back(displays[i]);
        }
        SDL_free(displays);
#else
        for (int i = 0; i < GetMonitorCount(); i++)
        {
            LIST.push_back(i);
        }
#endif
        return LIST;
    }

    void DrawTruePixelartScale(RenderTexture texture)
    {
        auto canvas = Point{(float)texture.texture.width, (float)texture.texture.height};
        const auto display = GetScreenDims();
        auto scale = display / canvas;
        if (scale <= 0)
        {
            scale = {1, 1};
        }
        // Use for both axis the smallest common scalar
        scale.y = std::min(scale.x, scale.y);
        scale.x = std::min(scale.x, scale.y);
        canvas *= scale;
        canvas.floor();
        SetMouseScale(1.0F / scale.x, 1.0F / scale.y);
        const auto drawPos = Rect::CenteredIn(canvas, display).pos();
        SetMouseOffset((int)-drawPos.x, (int)-drawPos.y);
        DrawRenderTexture(texture, drawPos, scale);
    }

    Point GetScreenDims() { return {(float)GetScreenWidth(), (float)GetScreenHeight()}; }

    Point GetWorldToScreen2DNorm(const Point world, Point screen)
    {
        const auto pos = Point{GetWorldToScreen2D(world, CameraGet())};
        return pos / screen;
    }

    Point GetGamePadLeftStick(int gamepad, float deadZone)
    {
        Point p{GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X),
                GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y)};
        if (p.abs().sum() < deadZone)
            return {};
        return p;
    }

    Point GetGamePadRightStick(int gamepad, float deadZone)
    {
        Point p{GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X),
                GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y)};
        if (p.abs().sum() < deadZone)
            return {};
        return p;
    }

    RenderTextureWrapper::RenderTextureWrapper(const RenderTexture& texture, const RenderTexture& old) : old(&old)
    {
        BeginTextureMode(texture);
    }

    RenderTextureWrapper::~RenderTextureWrapper()
    {
        EndTextureMode();
        if (old != nullptr && old->texture.id != 0)
            BeginTextureMode(*old);
    }


} // namespace magique
