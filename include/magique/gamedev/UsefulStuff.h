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


    // A shader that swaps a given color to a target color - the color must match exactly
    // Useful for reskinning textures
    struct ColorSwapShader final
    {
        struct ColorPair
        {
            Color from{};
            Color to{};
        };

        // Activates the shader and swaps up 4 colors - using BeginShaderMode
        ColorSwapShader(const std::array<ColorPair, 4>& pairs);
        // Ends the shader using EndShaderMode
        ~ColorSwapShader();

    private:
        static void Init();
        friend struct Game;
    };


} // namespace magique

#endif // MAGEQUEST_USEFULSTUFF_H
