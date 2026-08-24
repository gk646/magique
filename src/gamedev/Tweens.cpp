#include <cmath>
#include <algorithm>

#include "internal/globals/TweenData.h"
#ifndef PI
#define PI 3.14159265358979323846f
#endif
namespace magique
{
    Tween::Tween(TweenMode mode, float seconds) : forwardMode(mode), backwardMode(mode) { setDuration(seconds); }

    Tween::Tween(TweenMode forward, TweenMode backward, float seconds) : forwardMode(forward), backwardMode(backward)
    {
        setDuration(seconds);
    }

    void Tween::reset()
    {
        global::TWEEN_DATA.remove(*this);
        started = false;
        step = 0.0F;
    }

    void Tween::forward()
    {
        isForward = true;
        if (!started)
        {
            global::TWEEN_DATA.add(*this);
            started = true;
        }
    }

    void Tween::backward()
    {
        isForward = false;
        if (!started)
        {
            global::TWEEN_DATA.add(*this);
            started = true;
        }
    }

    void Tween::setOnTick(const std::function<void(const Tween&)>& newTickFunc) { this->tickFunc = newTickFunc; }

    void Tween::setDuration(const float seconds) { stepWidth = 1.0F / (MAGIQUE_LOGIC_TICKS * seconds); }

    float Tween::getValue() const
    {
        constexpr float c4 = (2 * PI) / 3;
        switch (getMode())
        {
        case TweenMode::IN_OUT_SINE:
            return -(std::cos(PI * step) - 1.0F) / 2.0F;
        case TweenMode::LINEAR:
            return step;
        case TweenMode::IN_OUT_CUBIC:
            return step < 0.5F ? 4.0F * step * step * step : 1 - std::pow(-2.0F * step + 2.0F, 3.0F) / 2.0F;
        case TweenMode::IN_OUT_QUAD:
            return step < 0.5 ? 2 * step * step : 1 - std::pow(-2.0F * step + 2.0F, 2.0F) / 2.0F;
        case TweenMode::IN_OUT_CIRC:
            return step < 0.5 ? (1.0F - std::sqrtf(1.0F - std::pow(2.0F * step, 2.0F))) / 2.0F
                              : (std::sqrtf(1.0F - std::pow(-2.0F * step + 2.0F, 2.0F)) + 1.0F) / 2.0F;
        case TweenMode::IN_QUINT:
            return step * step * step * step;
        case TweenMode::OUT_SINE:
            return std::sin((step * PI) / 2.0F);
        case TweenMode::OUT_ELASTIC:
            return step == 0 ? 0 : step == 1 ? 1 : -std::pow(2, -10 * step) * sin((step * 10 - 0.75F) * c4) + 1;
        case TweenMode::OUT_QUINT:
            return 1.0F - std::pow(1.0F - step, 5);
        }
        return 0.0F;
    }

    float Tween::getStep() const { return step; }

    void Tween::setStep(float value) { step = std::clamp(value, 0.0F, 1.0F); }

    bool Tween::isDone() const { return (isForward && step >= 1.0F) || (!isForward && step <= 0.0F); }

    bool Tween::isStarted() const { return started; }

    TweenMode Tween::getMode() const { return isForward ? forwardMode : backwardMode; }

    void Tween::update()
    {
        if (isForward)
        {
            step = std::min(1.0F, step + stepWidth);
        }
        else
        {
            step = std::max(0.0F, step - stepWidth);
        }
    }

} // namespace magique
