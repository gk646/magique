// SPDX-License-Identifier: zlib-acknowledgement
#include <algorithm>
#include <raylib/raylib.h>

#include <magique/ui/controls/Slider.h>
#include <magique/ui/UI.h>
#include <magique/util/Math.h>

#include "internal/globals/EngineConfig.h"
#include "magique/util/RayUtils.h"

namespace magique
{
    Slider::Slider(Rect bounds, std::string_view label, Direction labelDir, Anchor anchor, Point inset,
                   ScalingMode scaling) : LabelledObject(bounds, label, labelDir, anchor, inset, scaling)
    {
        setGamepadMapping(new GamepadMapping(*this,
                                             [&](GamepadMappingState& state, GamepadButton button)
                                             {
                                                 int direction = state.isLeft() ? -1 : 1;
                                                 setSliderPercent(sliderPos + 0.05F * direction);
                                                 return Point{-1};
                                             }));
    }

    void Slider::setScaleLinear(const float min, const float max)
    {
        this->min = min;
        this->max = max;
        this->mid = min + (max - min) / 2;
        mode = SliderMode::Linear;
    }

    void Slider::setScaleZones(const float min, const float mid, const float max)
    {
        this->min = min;
        this->mid = mid;
        this->max = max;
        mode = SliderMode::TwoZones;
    }

    float Slider::getSliderValue() const
    {
        if (mode == SliderMode::Linear)
        {
            return std::lerp(min, max, sliderPos);
        }

        if (mode == SliderMode::TwoZones)
        {
            if (sliderPos < 0.5)
            {
                const auto newPos = MathLerpInverse(0.0F, 0.5F, sliderPos);
                return std::lerp(min, mid, newPos);
            }
            const auto newPos = MathLerpInverse(0.5F, 1.0F, sliderPos);
            return std::lerp(mid, max, newPos);
        }
        return 0.0F;
    }

    float Slider::getSliderPercent() const { return sliderPos; }

    void Slider::setSliderPercent(const float value, bool forceCallback)
    {
        const float prev = sliderPos;
        sliderPos = std::clamp(value, 0.0F, 1.0F);
        if ((forceCallback || sliderPos != prev) && func)
            func(getSliderValue(), sliderPos);
    }

    void Slider::setOnChange(const SliderChangeFunc& newFunc) { func = newFunc; }

    void Slider::updateActions(const Rect& bounds)
    {
        if (getIsHovered())
        {
            const auto knob = getKnob();
            const auto hovered = knob.contains(UIGetDragStart());

            // Ensure click started within knob
            if (!isDragged && hovered)
            {
                for (int i = 0; i < MOUSE_BUTTON_BACK + 1; ++i) // All mouse buttons
                {
                    if (IsMouseButtonReleased(i))
                    {
                        onClick(bounds, i);
                    }
                }
                isDragged = true;
                dragStartVal = sliderPos;
            }

            if (LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                setSliderPercent((GetMousePos().x - bounds.x) / bounds.width);
            }
        }

        if (isDragged && LayeredInput::IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            const auto diff = GetMousePos().x - UIGetDragStart().x;
            setSliderPercent(dragStartVal + diff / bounds.width);
            LayeredInput::ConsumeMouse();
        }
        else
        {
            isDragged = false;
        }
    }

    void Slider::drawDefault(const Rect& bounds) const
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const auto knob = getKnob();
        const Color body = theme.getBodyColor(getIsHovered(), getIsPressed());
        const Color outline = theme.getOutlineColor(getIsHovered(), getIsPressed());

        DrawRectFrameFilled(Rect::CenteredOn(bounds.mid(), {bounds.width, bounds.height * 0.5F}), theme.background,
                            theme.backOutline);
        DrawRectFrameFilled(knob, body, outline);
    }

    Rect Slider::getKnob() const
    {
        const auto bounds = getBounds();
        return Rect::CenteredOn({bounds.x + sliderPos * bounds.width, bounds.y + bounds.height * 0.5F},
                                {bounds.width * 0.1F, bounds.height});
    }


} // namespace magique
