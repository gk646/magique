#include <raylib/raylib.h>

#include <magique/ui/controls/Button.h>
#include <magique/ui/UI.h>

#include "internal/headers/CollisionPrimitives.h"

namespace magique
{
    Button::Button(const float x, const float y, const float w, const float h) : UIObject(x, y, w, h) {}

    void Button::updateButtonActions(const Rectangle& bounds)
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
                    if (IsMouseButtonReleased(i))
                    {
                        onClick(bounds, i);
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
        const Color body = isHovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? DARKGRAY : isHovered ? GRAY : LIGHTGRAY;
        const Color outline = isHovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? GRAY : isHovered ? DARKGRAY : GRAY;
        DrawRectangleRounded(bounds, 0.1F, 20, body);
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 20, 2, outline);
    }


} // namespace magique