#ifndef MAGIQUE_BASESHADERS_H
#define MAGIQUE_BASESHADERS_H

#include <raylib/raylib.h>
#include <magique/util/RayUtils.h>

namespace magique
{

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

    // Applies a vignette effect to the target area
    struct VignetteShader final
    {
        operator Shader() const;

        // Default: RED
        VignetteShader& setColor(Color color);

        // Default: 0.15
        VignetteShader& setFalloff(float falloff);

        // Default: 0.55
        VignetteShader& setSize(float size);

    private:
        M_MAKE_PUB()
        static void Init();
        static int colorLoc;
        static int fallOffLoc;
        static int sizeLoc;
        static Shader shader;
    };

    // Tries to draw an outline around the target area
    // Outline pixel happens if any pixel left/right/up/down is not transparent
    struct OutlineShader final
    {
        operator Shader() const;

        // Sets the color to use for the outline
        OutlineShader& setColor(Color color = RED);

        // Sets the thickness
        OutlineShader& setSize(float size = 1);

    private:
        M_MAKE_PUB()
        static void Init();
        static int colorLoc;
        static int sizeLoc;
        static Shader shader;
    };

} // namespace magique

#endif // MAGIQUE_BASESHADERS_H
