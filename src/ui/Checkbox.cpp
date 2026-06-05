#include <magique/ui/controls/Checkbox.h>
#include <magique/core/Engine.h>
#include <magique/ui/UI.h>
#include <magique/util/RayUtils.h>
#include <magique/core/Draw.h>

namespace magique
{

    CheckBox::CheckBox(Rect bounds, std::string_view label, Direction labelDir, Anchor anchor, Point inset,
                       ScalingMode scaling) : LabelledObject(bounds, label, labelDir, anchor, inset, scaling)
    {
    }

    void CheckBox::setOnChange(const ButtonChangeFunc& clickFunc) { func = clickFunc; }

    bool CheckBox::getState() const { return state; }

    void CheckBox::setState(bool newState, bool triggerCallback)
    {
        state = newState;
        if (triggerCallback && func)
            func(state);
    }


    void CheckBox::updateInputs()
    {
        if (getIsHovered() && LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            state = !state;
            if (func)
            {
                func(state);
            }
        }
    }

    void CheckBox::drawDefault(const Rect& bounds) const
    {
        const auto theme = EngineGetTheme();
        Color base = getIsHovered() ? theme.backHighlight : BLANK;
        DrawRectFrameFilled(bounds, base, theme.backOutline);
        if (getState())
        {
            const auto len = std::min(bounds.width, bounds.height) / 2 * 0.7F;
            DrawCircleV(bounds.mid(), len, theme.text);
        }
    }

} // namespace magique
