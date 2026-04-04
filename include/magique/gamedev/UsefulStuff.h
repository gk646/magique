#ifndef MAGEQUEST_USEFULSTUFF_H
#define MAGEQUEST_USEFULSTUFF_H

#include <array>
#include <raylib/raylib.h>

//===============================================
// Useful Stuff
//===============================================
// .....................................................................
// A collection of small useful stuff that wouldn't warrant a separate header on its own
// .....................................................................

namespace magique
{

    // A simple counter struct to clean up counter code
    struct Counter final
    {
        Counter(float goal, float step = 1, float begin = 0);

        // Update the count -  returns true if the goal is reached AND resets back to 0 ELSE return false
        // update() + isGoalReached() + reset()
        bool tick();

        // Increases the counter by "step"
        void update();

        // Returns true if goal is reached
        bool isGoalReached() const;

        void setStep(float newStep);
        float getStep() const;

        void setBegin(float newBegin);
        float getBegin() const;

        float getCount() const;

        // Sets count to goal such that tick() returns true on next call
        void fill();

        // Sets count to begin
        void reset();

    private:
        float count = 0;
        float goal;
        float begin;
        float step;
    };

} // namespace magique

#endif // MAGEQUEST_USEFULSTUFF_H
