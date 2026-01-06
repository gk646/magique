#ifndef MAGEQUEST_USEFULSTUFF_H
#define MAGEQUEST_USEFULSTUFF_H

#include <cstdint>

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
        Counter(uint32_t goal, uint32_t step = 1);

        // update() + isGoalReached()
        bool tick();

        // Increases the counter by "step"
        void update();

        // Returns true if goal is reached - resets count to 0 if true
        bool isGoalReached();

        void setStep(uint32_t newStep);
        uint32_t getStep();

        // Sets count to goal such that tick() returns true on next call
        void fill();

    private:
        uint32_t count = 0;
        uint32_t goal;
        uint32_t step;
    };

} // namespace magique

#endif //MAGEQUEST_USEFULSTUFF_H
