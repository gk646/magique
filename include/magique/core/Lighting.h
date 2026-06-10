#ifndef MAGIQUE_LIGHTING_H
#define MAGIQUE_LIGHTING_H

#include <raylib/raylib.h>

//===============================================
// Lighting
//===============================================
// .....................................................................
// magique has built-in lighting using polar-raytracing - this is VERY fast
// It brings nice lighting with soft shadows for every shape
// Performance scales with the radius of the light (and the amount) - the larger the slower
// Note: This system uses texture slot 4 and 5 (rlActiveTextureSlot(int))
//      - Only important if you manually use them or use a function that does such as DrawMesh()
// Note: The maximum amount of lights currently possible is 64 and max radius of light is 651
// .....................................................................

namespace magique
{

    // Draws raytraced lighting using LightingC components present - only for the camera area
    //      - texture: if specified will be drawn to the given texture else directly to the implicit framebuffer
    void LightingDrawRaytracing(RenderTexture texture = {});

    // Returns a texture in the dimension of camera rect contains black pixels if the pixel is not pass-through (solid)
    // This combines pathfinding grid data with OccluderC entities
    // Note: Uses and activates a render texture => you NEED to reactivate any render texture previously active after the call
    // Note: Used for built-in lighting but also useful for custom lighting
    // Note: This call is lazy (only updated when called) and is cached (updated at most once per tick regardless of calls)
    Texture2D LightingGetScreenOcclusion();

} // namespace magique


// IMPLEMENTATION


namespace magique
{
    namespace internal
    {
        // Called automatically
        void LightingInit();
    } // namespace internal
} // namespace magique

#endif // MAGIQUE_LIGHTING_H
