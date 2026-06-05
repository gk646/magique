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

    void Slider::setSliderPercent(const float value, bool callback)
    {
        sliderPos = value;
        if (callback && func)
            func(getSliderValue(), sliderPos);
    }

    void Slider::setOnChange(const SliderChangeFunc& newFunc) { func = newFunc; }

    void Slider::updateActions(const Rect& bounds)
    {
        if (getIsHovered())
        {
            const auto dragStart = UIGetDragStart();
            const auto sliderKnob = getKnobPosition();
            const auto radius = UIGetScaled(15);
            const auto knobVec = Vector2{sliderKnob.x, sliderKnob.y};
            const auto dragStarted = CheckCollisionPointCircle(Vector2{dragStart.x, dragStart.y}, knobVec, radius);

            // Ensure click started within knob
            if (!isDragged && dragStarted)
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
        }

        if (isDragged && LayeredInput::IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            const float prev = sliderPos;
            const auto diff = GetMousePos().x - UIGetDragStart().x;
            sliderPos = std::clamp(dragStartVal + diff / bounds.width, 0.0F, 1.0F);
            if (sliderPos != prev && func)
                func(getSliderValue(), sliderPos);
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
        Color body = theme.getBodyColor(getIsHovered(), getIsPressed());
        Color outline = theme.getOutlineColor(getIsHovered(), getIsPressed());
        const auto bodyHeight = bounds.height / 4.0F;

        outline.a = 150;
        const Rectangle sliderBody = {bounds.x, bounds.y + bodyHeight, bounds.width, bodyHeight * 2};
        DrawRectangleRounded(sliderBody, 0.5F, 20, outline);

        outline.a = 30;
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 20, 2, outline);

        const Vector2 sliderKnob = {bounds.x + sliderPos * bounds.width, sliderBody.y + sliderBody.height / 2.0F};
        DrawCircleV(sliderKnob, bounds.height / 2.0F, body);
    }

    Point Slider::getKnobPosition() const
    {
        const auto bounds = getBounds();
        return Point{bounds.x + sliderPos * bounds.width, bounds.y + bounds.height / 2.0F};
    }


} // namespace magique
