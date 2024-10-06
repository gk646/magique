#include <magique/ui/controls/Window.h>
#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"

namespace magique
{

    Window::Window(const float x, const float y, const float w, const float h, const float moverHeight) :
        UIContainer(x, y, w, h)
    {
        moverHeightP = moverHeight / MAGIQUE_UI_RESOLUTION_Y;
    }

    Rectangle Window::getBodyBounds() const
    {
        const auto [sx, sy] = global::UI_DATA.getScreenDims();
        Rectangle bounds{px, py + moverHeightP, pw, ph - moverHeightP};
        switch (scaleMode)
        {
        case ScalingMode::FULL:
            bounds.x *= sx;
            bounds.y *= sy;
            bounds.width *= sx;
            bounds.height *= sy;
            break;
        case ScalingMode::KEEP_RATIO:
            bounds.x *= sx;
            bounds.y *= sy;
            bounds.width = pw * MAGIQUE_UI_RESOLUTION_X / MAGIQUE_UI_RESOLUTION_Y * sy;
            bounds.height *= sy;
            break;
        case ScalingMode::NONE:
            bounds.x *= MAGIQUE_UI_RESOLUTION_X;
            bounds.y *= MAGIQUE_UI_RESOLUTION_Y;
            bounds.width *= MAGIQUE_UI_RESOLUTION_X;
            bounds.height *= MAGIQUE_UI_RESOLUTION_Y;
            break;
        }

        if (anchor != AnchorPosition::NONE)
        {
            const auto pos = GetUIAnchor(anchor, bounds.width, bounds.height);
            bounds.x = pos.x;
            bounds.y = pos.y;
        }
        return bounds;
    }

    Rectangle Window::getTopBarBounds() const
    {
        const auto [sx, sy] = global::UI_DATA.getScreenDims();
        Rectangle bounds{px, py, pw, moverHeightP};
        switch (scaleMode)
        {
        case ScalingMode::FULL:
            bounds.x *= sx;
            bounds.y *= sy;
            bounds.width *= sx;
            bounds.height *= sy;
            break;
        case ScalingMode::KEEP_RATIO:
            bounds.x *= sx;
            bounds.y *= sy;
            bounds.width = pw * MAGIQUE_UI_RESOLUTION_X / MAGIQUE_UI_RESOLUTION_Y * sy;
            bounds.height *= sy;
            break;
        case ScalingMode::NONE:
            bounds.x *= MAGIQUE_UI_RESOLUTION_X;
            bounds.y *= MAGIQUE_UI_RESOLUTION_Y;
            bounds.width *= MAGIQUE_UI_RESOLUTION_X;
            bounds.height *= MAGIQUE_UI_RESOLUTION_Y;
            break;
        }

        if (anchor != AnchorPosition::NONE)
        {
            const auto pos = GetUIAnchor(anchor, bounds.width, bounds.height);
            bounds.x = pos.x;
            bounds.y = pos.y;
        }
        return bounds;
    }

    void Window::drawDefault() const
    {
        const auto topBar = getTopBarBounds();
        if (CheckCollisionPointRec(GetMousePosition(), topBar))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) // Pressed
            {
                DrawRectangleRounded(topBar, 0.2F, 30, DARKGRAY);
                DrawRectangleRoundedLinesEx(topBar, 0.2F, 30, 2, GRAY);
            }
            else // Hovered
            {
                DrawRectangleRounded(topBar, 0.2F, 30, GRAY);
                DrawRectangleRoundedLinesEx(topBar, 0.2F, 30, 2, DARKGRAY);
            }
        }
        else // Idle
        {
            DrawRectangleRounded(topBar, 0.2F, 30, LIGHTGRAY);
            DrawRectangleRoundedLinesEx(topBar, 0.2F, 30, 2, GRAY);
        }

        const auto body = getBodyBounds();
        DrawRectangleRounded(body, 0.2F, 30, LIGHTGRAY);
        DrawRectangleRoundedLinesEx(body, 0.2F, 30, 2, GRAY);
    }
} // namespace magique