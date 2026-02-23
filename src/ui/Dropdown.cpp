#include <magique/ui/controls/DropDown.h>
#include <magique/util/RayUtils.h>
#include <magique/ui/UI.h>

#include "internal/globals/EngineConfig.h"

namespace magique
{
    Dropdown::Dropdown(Rect bounds, Anchor anchor, Point inset, ScalingMode scaling) :
        UIObject(bounds, anchor, inset, scaling), list(bounds)
    {
    }

    bool Dropdown::getIsOpen() const { return isOpen; }

    ListMenu& Dropdown::getList() { return list; }

    void Dropdown::updateInputs()
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (getBounds().contains(GetMousePosition()))
            {
                isOpen = true;
                LayeredInput::ConsumeMouse();
            }
            else
            {
                isOpen = false;
            }
        }
    }

    void Dropdown::drawDefault(const Rect& bounds)
    {
        const auto theme = global::ENGINE_CONFIG.theme;
        const auto body = theme.getBodyColor(getIsHovered(), getIsPressed());

        DrawRectFrameFilled(bounds, body, theme.backOutline);

        float height = getStartBounds().height;
        int idx = list.getSelectedIndex();
        if (idx != -1)
        {
            auto& entry = list.entries[idx];
            if (list.drawFunc)
            {
                height = list.drawFunc(bounds.pos(), entry.text.c_str(), idx, false, false);
            }
            else
            {
                height = list.drawDefaultEntry(bounds.pos(), entry.text.c_str(), false, false);
            }
        }

        if (isOpen)
        {
            list.setPosition(bounds.pos() + Point{0, height + 1});
            DrawRectFrameFilled(list.getBounds().enlarge(2), theme.backActive, theme.textPassive);
            list.draw();
        }
    }

} // namespace magique
