#ifndef TWEENS_H
#define TWEENS_H

#include <magique/fwd.hpp>
#include <functional>

//===============================================
// Tween Module
//===============================================
// .....................................................................
// Tweens are functions that modulate between 0 and 1 in some way
// They are very useful for transitions of any type:
//      - Camera zoom
//      - Transparency shift
// That show how a step translate to a value for different modes: https://easings.net/
// IMPORTANT: Tweens are registered internally and updated at the beginning of each tick and MUST NOT go out of scope
// .....................................................................

namespace magique
{

    enum class TweenMode : uint8_t
    {
        LINEAR,
        IN_OUT_CUBIC,
        IN_OUT_SINE,
        IN_OUT_QUAD,
        IN_OUT_CIRC,
    };

    struct Tween final
    {
        using TweenFunc = std::function<void(const Tween&)>;

        explicit Tween(TweenMode mode, float seconds = 1.0F);

        // Resets the step value to 0 and removes internally so its not updated anymore
        void reset();

        // Starts the tween (if it's not already started) and sets the update direction
        // forward: 0 -> 1 || reverse: 1 -> 0 (only sets direction, values are not changed)
        // Note: This tween object MUST NOT go out of scope or be deleted!
        void forward();
        void reverse();

        // Called each update tick with the tween
        void setOnTick(const TweenFunc& callback);

        // Sets the time it takes in seconds how long to reach the end
        void setDuration(float seconds);

        // Returns the current value of the tween
        float getValue() const;

        // Returns or sets the step of the tween [0, 1]
        float getStep() const;
        void setStep(float value);

        // Returns true if the step value reached its goal (1 if forward | 0 if reverse)
        bool isDone() const;

        // Returns true if the tween was started (and is getting updated)
        bool isStarted() const;

    private:
        void update();
        TweenFunc tickFunc;
        float step = 0.0F;
        float stepWidth = 1.0F / MAGIQUE_LOGIC_TICKS;
        bool started = false;
        bool isForward = true;
        TweenMode mode;
        friend TweenData;
    };
} // namespace magique

#endif // TWEENS_H
