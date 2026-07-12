#include <magique/ui/controls/DropDown.h>
#include <magique/util/RayUtils.h>
#include <magique/ui/UI.h>

#include "internal/globals/EngineConfig.h"

namespace magique
{
    Dropdown::Dropdown(Rect bounds, std::string_view label, Direction labelDir, Anchor anchor, Point inset,
                       ScalingMode scaling) :
        LabelledObject(bounds, label, labelDir, anchor, inset, scaling), list(bounds)
    {
        setGamepadMapping(new GamepadMapping(*this,
                                             [&](GamepadMappingState& state, GamepadButton button)
                                             {
                                                 getList().getGamepadMapping()->triggerEvent(state.event);
                                                 if (state.event == GamepadMappingEvent::Back)
                                                     isOpen = false;
                                                 if (isOpen && state.event == GamepadMappingEvent::Submit)
                                                     isOpen = false;
                                                 return Point{-1};
                                             }));
    }

    bool Dropdown::getIsOpen() const { return isOpen; }

    ListChooser& Dropdown::getList() { return list; }

    void Dropdown::updateInputs()
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (!LayeredInput::GetIsMouseConsumed() && getBounds().contains(GetMousePosition()))
            {
                UISetGamepadMap(getGamepadMapping());
                isOpen = !isOpen;
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
                height = list.drawDefaultEntry(bounds.pos(), entry.text.c_str(), isOpen || getIsHovered(), false);
            }
        }

        if (isOpen)
        {
            list.setPosition(bounds.pos() + Point{0.0F, height + 1});
            DrawRectFrameFilled(list.getBounds().enlarge(2), theme.backHighlight, theme.textPassive);
            list.draw();
        }
    }

} // namespace magique
