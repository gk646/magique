// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/controls/Window.h>
#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"
#include "internal/globals/EngineConfig.h"
#include "internal/utils/CollisionPrimitives.h"

namespace magique
{
    Window::Window(const float x, const float y, const float w, const float h, const float moverHeight) :
        UIObject(x, y, w, h)
    {
        const auto& ui = global::UI_DATA;
        if (moverHeight == 0.0F)
        {
            moverHeightP = (h * 0.10F) / ui.sourceRes.y;
        }
        else
        {
            moverHeightP = moverHeight / ui.sourceRes.y;
        }
    }

    bool Window::updateDrag(const Rectangle& area, const int mouseButton)
    {
        const auto mouse = GetMousePos();
        if (LayeredInput::IsMouseButtonDown(mouseButton))
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
                clickOffset.floor();
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
        const auto& ui = global::UI_DATA;
        Rectangle bounds{px, py + moverHeightP, pw, ph - moverHeightP};
        ui.scaleBounds(bounds, scaleMode, inset, anchor);
        return bounds;
    }

    Rectangle Window::getTopBarBounds() const
    {
        const auto& ui = global::UI_DATA;
        Rectangle bounds{px, py, pw, moverHeightP};
        ui.scaleBounds(bounds, scaleMode, inset, anchor);
        return bounds;
    }

    bool Window::getIsDragged() const { return isDragged; }

    void Window::drawDefault(const Rectangle& bounds) const
    {
        const auto& theme = global::ENGINE_CONFIG.theme;

        // Body
        DrawRectangleRounded(bounds, 0.1F, 30, theme.backLight);
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 30, 2, theme.backDark);

        const auto mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        // Top bar
        const auto topBar = getTopBarBounds();
        const auto isHovered = CheckCollisionPointRec(GetMousePos().v(), topBar) || isDragged;
        const Color body = isHovered && mouseDown ? theme.backSelected : isHovered ? theme.backLight : theme.backDark;
        const Color outline = isHovered && mouseDown ? theme.backLight : isHovered ? theme.backDark : theme.backDark;
        DrawRectangleRounded(topBar, 0.2F, 30, body);
        DrawRectangleRoundedLinesEx(topBar, 0.1F, 30, 2, outline);
    }
} // namespace magique