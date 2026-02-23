#include <algorithm>

#include <magique/gamedev/UsefulStuff.h>

namespace magique
{
    Counter::Counter(float goal, float step, float begin) : count(begin), goal(goal), begin(begin), step(step) {}

    bool Counter::tick()
    {
        update();
        if (isGoalReached())
        {
            reset();
            return true;
        }
        return false;
    }

    void Counter::update()
    {
        if (step > 0)
        {
            count = std::min(count + step, goal);
        }
        else
        {
            count = std::max(count + step, goal);
        }
    }

    bool Counter::isGoalReached() const
    {
        return count == goal;
    }

    void Counter::setStep(float newStep) { step = newStep; }

    float Counter::getStep() const { return step; }

    void Counter::setBegin(float newBegin) { begin = newBegin; }

    float Counter::getBegin() const { return begin; }

    float Counter::getCount() const { return count; }

    void Counter::fill() { count = goal; }

    void Counter::reset() { count = begin; }

    inline Shader COLOR_SWAP{};

    ColorSwapShader::ColorSwapShader(const std::array<ColorPair, 4>& pairs) {}

    ColorSwapShader::~ColorSwapShader() {}

    void ColorSwapShader::Init() {}

} // namespace magique
