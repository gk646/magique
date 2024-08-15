#include <magique/ui/controls/Button.h>

namespace magique
{
    Button::Button(const float x, const float y, const float w, const float h, const UILayer layer) :
        UIObject(x, y, w, h)
    {
    }

    void Button::draw(const Rectangle& bounds)
    {
        if (wasHovered) // 1 Tick delayed but saves us the lookup
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                drawClicked(bounds);
            }
            else
            {
                drawHovered(bounds);
            }
        }
        else
        {
            drawIdle(bounds);
        }
    }

    void Button::update(const Rectangle& bounds, bool isDrawn)
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