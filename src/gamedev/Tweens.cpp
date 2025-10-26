#include <cmath>

#include "internal/globals/TweenData.h"
#ifndef PI
#define PI 3.14159265358979323846f
#endif
namespace magique
{
    Tween::Tween(TweenMode mode, float seconds) : mode(mode) { setDuration(seconds); }

    float Tween::getValue() const
    {
        switch (mode)
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
            return step < 0.5 ? (1.0F - std::sqrt(1.0F - std::pow(2.0F * step, 2.0F))) / 2.0F
                              : (std::sqrt(1.0F - std::pow(-2.0F * step + 2.0F, 2.0F)) + 1.0F) / 2.0F;
        }
        return 0.0F;
    }

    float Tween::getStep() const { return step; }

    void Tween::reset()
    {
        global::TWEEN_DATA.remove(*this);
        started = false;
        step = 0.0F;
    }

    void Tween::start()
    {
        if (!started)
        {
            global::TWEEN_DATA.add(*this);
            started = true;
        }
    }

    void Tween::setOnTick(const std::function<void(const Tween&)>& newTickFunc) { this->tickFunc = newTickFunc; }

    void Tween::setDuration(const float seconds) { stepWidth = 1.0F / (MAGIQUE_LOGIC_TICKS * seconds); }

    bool Tween::isDone() const { return step >= 1.0F; }

    bool Tween::isStarted() const { return started; }

} // namespace magique
