// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/controls/Window.h>

#include "internal/globals/UIData.h"
#include "internal/globals/EngineConfig.h"

namespace magique
{
    Window::Window(const char* name, Rect bounds, const float moverHeight) : UIObject(bounds), name(name)
    {
        const auto& ui = global::UI_DATA;
        if (moverHeight == 0.0F)
        {
            moverHeightP = (bounds.height * 0.10F) / ui.sourceRes.y;
        }
        else
        {
            moverHeightP = moverHeight / ui.sourceRes.y;
        }
    }

    void Window::drawDefault(const Rect& bounds) const
    {
        const auto& theme = global::ENGINE_CONFIG.theme;

        // Body
        DrawRectFrameFilled(bounds, theme.background, theme.backOutline);
        const auto mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        // Top bar
        const auto topBar = getTopBarBounds();
        const auto isHovered = CheckCollisionPointRec(GetMousePos(), topBar) || isDragged;
        Color body = theme.getBodyColor(isHovered, isHovered && mouseDown);
        Color outline = theme.getOutlineColor(isHovered, isHovered && mouseDown);
        DrawRectangleRounded(topBar, 0.2F, 30, body);
        DrawRectangleRoundedLinesEx(topBar, 0.1F, 30, 2, outline);
    }
    std::string& Window::getName() { return name; }

    const char* Window::getName() const { return name.c_str(); }

    Rectangle Window::getBodyBounds() const
    {
        const auto& ui = global::UI_DATA;
        Rect bounds{pBounds.x, pBounds.y + moverHeightP, pBounds.width, pBounds.height - moverHeightP};
        ui.scaleBounds(bounds, scaleMode, inset, anchor);
        return bounds;
    }

    Rectangle Window::getTopBarBounds() const
    {
        const auto& ui = global::UI_DATA;
        Rect bounds = pBounds;
        bounds.height = moverHeightP;
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
