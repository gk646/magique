#ifndef TWEENS_H
#define TWEENS_H

#include <magique/fwd.hpp>
#include <functional>

//===============================================
// Tween Module
//===============================================
// .....................................................................
// https://easings.net/
// Tweens are functions that modulate between 0 and 1 in some way
// They are very useful for transitions of any type:
//      - Camera zoom
//      - Transparency shift
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

        // Returns the current value of the tween
        float getValue() const;

        // Returns the current step value - from 0.0 - 1.0
        float getStep() const;

        // Resets the step value to 0 and removes internally so its not updated anymore
        void reset();

        // Starts the tween - if already started its reset
        // Note: This tween object MUST NOT go out of scope or be deleted!
        void start();

        // Called each update tick with the tween
        void setOnTick(const TweenFunc& callback);

        // Sets the time it takes in seconds how long to reach the end
        void setDuration(float seconds);

        //================= UTIL =================//

        // Returns true if the step value is 1.0
        bool isDone() const;

        // Returns true if the tween was started
        bool isStarted() const;

    private:
        TweenFunc tickFunc;
        float step = 0.0F;
        float stepWidth = 1.0F / MAGIQUE_LOGIC_TICKS;
        bool started = false;
        TweenMode mode;
        friend TweenData;
    };
} // namespace magique

#endif //TWEENS_H
