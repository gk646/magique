#include <magique/ui/controls/Checkbox.h>

#include "magique/core/Engine.h"
#include "magique/core/Draw.h"
#include "magique/ui/UI.h"
#include "magique/util/RayUtils.h"
#include "raylib/raylib.h"

namespace magique
{

    CheckBox::CheckBox(float x, float y, float w, float h) : UIObject(x, y, w, h) {}

    CheckBox::CheckBox(float w, float h, Anchor anchor, Point inset) : UIObject(w, h, anchor, inset) {}

    void CheckBox::setOnClick(const SwitchFunc& clickFunc) { func = clickFunc; }

    bool CheckBox::getState() const { return state; }

    void CheckBox::setState(bool newState) { state = newState; }

    const std::string& CheckBox::getInfoText() const { return infoText; }

    void CheckBox::setInfoText(const std::string& text) { infoText = text; }

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

    void CheckBox::drawDefault(const Rectangle& bounds) const
    {
        Color base = getIsHovered() ? DARKGRAY : BLANK;
        DrawRectFrameFilled(bounds, base, DARKGRAY);
        Rect rect{bounds};
        if (getState())
        {
            const auto len = std::min(bounds.width, bounds.height) / 2 * 0.7F;
            DrawCircleV(rect.mid(), len, YELLOW);
        }
        const auto size = UIGetScaled(15);
        const auto pos = Point{bounds.x - 2, bounds.y + (bounds.height - size) / 2};
        DrawTextRightBound(EngineGetFont(), infoText.c_str(), pos, size);
    }

} // namespace magique
