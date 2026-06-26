#ifndef MAGIQUE_LIGHTING_H
#define MAGIQUE_LIGHTING_H

#include <raylib/raylib.h>

//===============================================
// Lighting
//===============================================
// .....................................................................
// magique has built-in lighting using polar-raytracing - this is VERY fast
// It brings nice lighting with soft shadows and supports any occlusion shape
// Performance scales with the radius of the light (and the amount) - the larger the slower
// Note: This system uses texture slot 4 and 5 (rlActiveTextureSlot(int))
//      - Only important if you manually use them or use a function that does such as DrawMesh()
// Note: The maximum amount of lights currently possible is 64 and max radius of light is 651
// Note: Raytracing only supports circular lights currently!
// Note: Implementation was inspired by: https://github.com/Yaazarai/2D-QuickRayTracing-GLSL
// .....................................................................

namespace magique
{

    // Draws raytraced lighting using LightingC components - culled to the camera area
    //      - texture: if specified will be drawn to the given texture else directly to the implicit framebuffer
    void LightingDrawRaytracing(RenderTexture texture = {});

    // Draws simpler lighting
    void LightingDrawSimple(RenderTexture texture = {});

    // Returns a texture in the dimension of camera rect contains black pixels if the pixel is not pass-through (solid)
    // This combines pathfinding grid data with OccluderC entities
    // Note: Used for built-in lighting but also useful for custom lighting
    // Note: This call is LAZY (only generated when called) and CACHED (updates at most once per tick regardless of calls)
    Texture2D LightingGetScreenOcclusion();

} // namespace magique


// IMPLEMENTATION


namespace magique::internal
{
    // Called automatically
    void LightingInit();
} // namespace magique::internal

#endif // MAGIQUE_LIGHTING_H
