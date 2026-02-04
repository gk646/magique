// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/ui/controls/Slider.h>
#include <magique/ui/UI.h>
#include <magique/util/Math.h>

#include "internal/globals/EngineConfig.h"

namespace magique
{
    Slider::Slider(const float x, const float y, const float w, const float h) : UIObject(x, y, w, h) {}

    Slider::Slider(const float w, const float h, const Anchor anchor, const float inset) : UIObject(w, h, anchor, Point{inset})
    {
    }

    void Slider::setScaleBalanced(const float min, const float max)
    {
        this->min = min;
        this->max = max;
        this->mid = min + (max - min) / 2;
        mode = SliderMode::BALANCED;
    }

    void Slider::setScaleImblanced(const float min, const float mid, const float max)
    {
        this->min = min;
        this->mid = mid;
        this->max = max;
        mode = SliderMode::IMBALANCED;
    }

    float Slider::getSliderValue() const
    {
        if (mode == SliderMode::BALANCED)
        {
            return Lerp(min, max, sliderPos);
        }

        if (mode == SliderMode::IMBALANCED)
        {
            if (sliderPos < 0.5)
            {
                const auto newPos = LerpInverse(0.0F, 0.5F, sliderPos);
                return Lerp(min, mid, newPos);
            }
            const auto newPos = LerpInverse(0.5F, 1.0F, sliderPos);
            return Lerp(mid, max, newPos);
        }
        return 0.0F;
    }

    float Slider::getSliderPercent() const { return sliderPos; }

    void Slider::setSliderPercent(const float value) { sliderPos = value; }

    void Slider::updateActions(const Rectangle& bounds)
    {
        if (getIsHovered())
        {
            const auto dragStart = GetDragStartPosition();
            const auto sliderKnob = getKnobPosition();
            const auto radius = UIGetScaled(15);
            const auto knobVec = Vector2{sliderKnob.x, sliderKnob.y};
            const auto dragStarted = CheckCollisionPointCircle(Vector2{dragStart.x, dragStart.y}, knobVec, radius);

            // Ensure click started within knob
            if (dragStarted)
            {
                for (int i = 0; i < MOUSE_BUTTON_BACK + 1; ++i) // All mouse buttons
                {
                    if (IsMouseButtonReleased(i))
                    {
                        onClick(bounds, i);
                    }
                }
                isDragged = true;
            }


            if (!isHovered)
            {
                isHovered = true;
                onHover(bounds);
            }
        }
        else
        {
            isHovered = false;
        }

        if (isDragged && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            const auto delta = GetMouseDelta();
            sliderPos = clamp(sliderPos + delta.x / bounds.width, 0.0F, 1.0F);
        }
        else
        {
            isDragged = false;
        }
    }

    void Slider::drawDefault(const Rectangle& bounds) const
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const auto mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        const Color body = isHovered && mouseDown ? theme.backSelected : isHovered ? theme.backLight : theme.backDark;
        Color outline = isHovered && mouseDown ? theme.backLight : isHovered ? theme.backDark : theme.backLight;
        const auto bodyHeight = bounds.height / 4.0F;

        outline.a = 150;
        const Rectangle sliderBody = {bounds.x, bounds.y + bodyHeight, bounds.width, bodyHeight * 2};
        DrawRectangleRounded(sliderBody, 0.5F, 20, outline);

        outline.a = 30;
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 20, 2, outline);

        const Vector2 sliderKnob = {bounds.x + sliderPos * bounds.width, sliderBody.y + sliderBody.height / 2.0F};
        DrawCircleV(sliderKnob, UIGetScaled(15), body);
    }

    Point Slider::getKnobPosition() const
    {
        const auto bounds = getBounds();
        return Point{bounds.x + sliderPos * bounds.width, bounds.y + bounds.height / 2.0F};
    }


} // namespace magique
