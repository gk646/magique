#include <magique/gamedev/UsefulStuff.h>

namespace magique
{
    Counter::Counter(uint32_t goal, uint32_t step) : goal(goal), step(step) {}

    bool Counter::tick()
    {
        update();
        return isGoalReached();
    }

    void Counter::update() { count += step; }

    bool Counter::isGoalReached()
    {
        if (count >= goal)
        {
            count = 0;
            return true;
        }
        return false;
    }

    void Counter::setStep(uint32_t newStep) { step = newStep; }

    uint32_t Counter::getStep() { return step; }

    void Counter::fill() { count = goal; }

} // namespace magique
