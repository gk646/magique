#ifndef MAGEQUEST_USEFULSTUFF_H
#define MAGEQUEST_USEFULSTUFF_H

#include <array>
#include <cstdint>
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
        Counter(uint32_t goal, uint32_t step = 1);

        // update() + isGoalReached()
        bool tick();

        // Increases the counter by "step"
        void update();

        // Returns true if goal is reached - resets count to 0 if true
        bool isGoalReached();

        void setStep(uint32_t newStep);
        uint32_t getStep() const;

        // Sets count to goal such that tick() returns true on next call
        void fill();

        // Sets count to 0
        void reset();

    private:
        uint32_t count = 0;
        uint32_t goal;
        uint32_t step;
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
