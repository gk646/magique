// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/ui/controls/Button.h>
#include <magique/ui/UI.h>

#include "internal/utils/CollisionPrimitives.h"
#include "internal/globals/EngineConfig.h"

namespace magique
{
    Button::Button(const float x, const float y, const float w, const float h) : UIObject(x, y, w, h) {}

    Button::Button(const float w, const float h, const Anchor anchor, const float inset) : UIObject(w, h, anchor, inset)
    {
    }

    void Button::wireOnClick(ClickFunc func) { clickFunc = func; }

    void Button::setDisabled(bool value) { isDisabled = value; }

    bool Button::getIsDisabled() const { return isDisabled; }

    void Button::updateActions(const Rectangle& bounds)
    {
        const auto mouse = GetMousePos();
        const auto dragStart = GetDragStartPosition();
        if (PointToRect(mouse.x, mouse.y, bounds.x, bounds.y, bounds.width, bounds.height))
        {
            // Ensure click started within button
            if (PointToRect(dragStart.x, dragStart.y, bounds.x, bounds.y, bounds.width, bounds.height))
            {
                for (int i = 0; i < MOUSE_BUTTON_BACK + 1; ++i) // All mouse buttons
                {
                    if (IsMouseButtonReleased(i) && !isDisabled)
                    {
                        onClick(bounds, i);
                        if (clickFunc)
                        {
                            clickFunc(bounds, i);
                        }
                    }
                }
            }
            if (!isHovered)
            {
                isHovered = true;
                onHover(bounds);
            }
        }
        else
        {
            isHovered = false;
        }
    }

    void Button::drawDefault(const Rectangle& bounds) const
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const auto mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        const Color body = isHovered && mouseDown ? theme.backSelected : isHovered ? theme.backLight : theme.backDark;
        const Color outline = isHovered && mouseDown ? theme.backLight : isHovered ? theme.backDark : theme.backDark;
        DrawRectangleRounded(bounds, 0.1F, 20, body);
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 20, 2, outline);
    }


} // namespace magique