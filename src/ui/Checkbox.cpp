#include <magique/ui/controls/Checkbox.h>
#include <magique/core/Engine.h>
#include <magique/ui/UI.h>
#include <magique/util/RayUtils.h>
#include <magique/core/Draw.h>

namespace magique
{

    CheckBox::CheckBox(Rect bounds, Anchor anchor, Point inset, ScalingMode mode) : UIObject(bounds, anchor, inset, mode)
    {
    }

    void CheckBox::setOnChange(const ChangeFunc& clickFunc) { func = clickFunc; }

    bool CheckBox::getState() const { return state; }

    void CheckBox::setState(bool newState) { state = newState; }

    const std::string& CheckBox::getInfoText() const { return infoText; }

    Direction CheckBox::getInfoDirection() const { return infoDir; }

    void CheckBox::setInfoText(const std::string& text, Direction dir)
    {
        infoText = text;
        infoDir = dir;
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

        Point textPos = bounds.pos();
        const auto fSize = EngineGetFont().baseSize * (int)UIGetScaled(1);
        const auto dims = MeasureTextEx(EngineGetFont(), infoText.c_str(), fSize, 1.0F);
        switch (infoDir)
        {
        case Direction::LEFT:
            textPos -= Point{dims.x + 2, 0};
            break;
        case Direction::RIGHT:
            textPos += Point{dims.x + bounds.width + 2, 0};
            break;
        case Direction::UP:
            textPos -= Point{0, dims.y + 2};
            break;
        case Direction::DOWN:
            textPos += Point{0, bounds.height + 2};
            break;
        }
        DrawTextEx(EngineGetFont(), infoText.c_str(), textPos, fSize, 1.0F, theme.text);
    }

} // namespace magique
