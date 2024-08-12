#include <magique/ui/controls/Button.h>

namespace magique
{
    Button::Button(const GameState gameState, const float x, const float y, const float w, const float h,
                   const UILayer layer) : UIObject(gameState, x, y, w, h, layer)
    {
    }

    void Button::draw()
    {
        const auto bounds = getBounds();
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

    void Button::update()
    {
        const bool hovered = getIsHovered();
        if (hovered)
        {
            const auto bounds = getBounds();
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