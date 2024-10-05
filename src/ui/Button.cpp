#include <magique/ui/controls/Button.h>

namespace magique
{
    Button::Button(const float x, const float y, const float w, const float h) :
        UIObject(x, y, w, h)
    {
    }

    void Button::onUpdate(const Rectangle& bounds, bool isDrawn)
    {
        const bool hovered = getIsHovered();
        if (hovered)
        {
            if (!wasHovered)
            {
                onHover(bounds);
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                onClick(bounds);
            }
        }
        wasHovered = hovered;
    }

} // namespace magique