#ifndef MAGIQUE_BASESHADERS_H
#define MAGIQUE_BASESHADERS_H

#include <raylib/raylib.h>

namespace magique
{


    struct VignetteShader : ShaderWrapper
    {
        void setColor(Color color);
        void setFalloff(float falloff);
        void setSize(float size);
    };

} // namespace magique

#endif // MAGIQUE_BASESHADERS_H
