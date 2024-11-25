// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/controls/Window.h>
#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"
#include "internal/headers/CollisionPrimitives.h"

namespace magique
{

    Window::Window(const float x, const float y, const float w, const float h, const float moverHeight) :
        UIContainer(x, y, w, h)
    {
        if (moverHeight == 0.0F)
        {
            moverHeightP = (h * 0.10F) / MAGIQUE_UI_RESOLUTION_Y;
        }
        else
        {
            moverHeightP = moverHeight / MAGIQUE_UI_RESOLUTION_Y;
        }
    }

    bool Window::updateDrag(const Rectangle& area, const int mouseButton)
    {
        const auto mouse = global::UI_DATA.getMousePos();
        if (UIInput::IsMouseButtonDown(mouseButton))
        {
            if (isDragged)
            {
                setPosition(mouse.x + clickOffset.x, mouse.y + clickOffset.y);
                return true;
            }
            if (PointToRect(mouse.x, mouse.y, area.x, area.y, area.width, area.height))
            {
                const auto bounds = getBounds();
                clickOffset = {bounds.x - mouse.x, bounds.y - mouse.y};
                isDragged = true;
                return true;
            }
        }
        else
        {
            isDragged = false;
        }
        return false;
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
        if (anchor != Anchor::NONE)
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
        if (anchor != Anchor::NONE)
        {
            const auto pos = GetUIAnchor(anchor, bounds.width, bounds.height);
            bounds.x = pos.x;
            bounds.y = pos.y;
        }
        return bounds;
    }

    bool Window::getIsDragged() const { return isDragged; }

    void Window::drawDefault(const Rectangle& bounds) const
    {
        // Body
        DrawRectangleRounded(bounds, 0.1F, 30, LIGHTGRAY);
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 30, 2, GRAY);

        // Top bar
        const auto topBar = getTopBarBounds();
        const auto hovered = CheckCollisionPointRec(GetMousePosition(), topBar);
        const Color body = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? DARKGRAY : hovered ? GRAY : LIGHTGRAY;
        const Color outline = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? GRAY : hovered ? DARKGRAY : GRAY;
        DrawRectangleRounded(topBar, 0.2F, 30, body);
        DrawRectangleRoundedLinesEx(topBar, 0.1F, 30, 2, outline);
    }
} // namespace magique