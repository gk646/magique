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

    bool Window::updateDrag(const Rect& area, const int mouseButton)
    {
        const auto mouse = GetMousePos();
        const auto contained = area.contains(mouse);
        bool alreadyDragging = !IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isDragged;
        bool startDrag = LayeredInput::IsMouseButtonDown(mouseButton) && contained;
        bool isDragging = startDrag || alreadyDragging;

        if (isDragging)
        {
            if (!isDragged)
            {
                const auto bounds = getBounds();
                clickOffset = {bounds.x - mouse.x, bounds.y - mouse.y};
                clickOffset.floor();
                isDragged = true;
                return true;
            }
            else
            {
                setPosition(mouse + clickOffset);
                return true;
            }
        }
        else
        {
            isDragged = false;
        }
        return false;
    }
} // namespace magique
